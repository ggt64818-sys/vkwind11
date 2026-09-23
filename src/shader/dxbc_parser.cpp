#include "dxbc_parser.h"
#include "../common/logging.h"

// ============================================================================
// DXBC Parser Implementation
// ============================================================================
//
// DXBC format (chunk-based):
//   [FourCC "DXBC"] [uint32 checksum] [uint32 version] [uint32 size] [uint32 numChunks]
//   [chunk0 offset] [chunk1 offset] ...
//
// Each chunk:
//   [FourCC] [uint32 sizeInBytes] [data...]
//
// Key chunks:
//   SHDR - shader bytecode (SM4/SM5 instructions)
//   RDEF - resource definitions (cbuffers, textures)
//   ISGN - input signature
//   OSGN - output signature
//   PSNE - pixel shader signature
//   STAT - statistics
//

// FourCC codes
constexpr uint32_t FOURCC_DXBC = 0x43425844; // "DXBC"
constexpr uint32_t FOURCC_SHDR = 0x52444853; // "SHDR"
constexpr uint32_t FOURCC_RDEF = 0x46454452; // "RDEF"
constexpr uint32_t FOURCC_ISGN = 0x4E475349; // "ISGN"
constexpr uint32_t FOURCC_OSGN = 0x4E47534F; // "OSGN"
constexpr uint32_t FOURCC_STAT = 0x54415453; // "STAT"
constexpr uint32_t FOURCC_AON11 = 0x31314E4F; // "AON11" (D3D11 debug info)
constexpr uint32_t FOURCC_SPDB = 0x42445053; // "SPDB" (debug info)
constexpr uint32_t FOURCC_SFI0 = 0x30494653; // "SFI0" (feature info)

bool parseDXBC(const uint32_t* bytecode, size_t sizeBytes, ParsedDXBC& out) {
  if (!bytecode || sizeBytes < 24) {
    VKWIND11_LOG_ERROR("DXBC: invalid input");
    return false;
  }

  // Check DXBC magic
  if (bytecode[0] != FOURCC_DXBC) {
    VKWIND11_LOG_ERROR("DXBC: invalid magic: 0x%08x", bytecode[0]);
    return false;
  }

  // uint32 checksum at [1]
  // uint32 version at [2] — upper 16 bits = major, lower 16 bits = minor
  // uint32 totalSize at [3]
  // uint32 numChunks at [4]

  uint32_t version = bytecode[2];
  out.majorVersion = (version >> 16) & 0xFFFF;
  out.minorVersion = version & 0xFFFF;
  uint32_t numChunks = bytecode[4];

  VKWIND11_LOG_INFO("DXBC: version %u.%u, %u chunks", out.majorVersion, out.minorVersion, numChunks);

  // Store the full bytecode
  out.bytecode.assign(bytecode, bytecode + (sizeBytes / 4));

  // Parse chunks
  const uint32_t* chunkOffsets = &bytecode[5];

  for (uint32_t i = 0; i < numChunks && chunkOffsets[i] < sizeBytes / 4; i++) {
    uint32_t offset = chunkOffsets[i];
    if (offset + 2 > sizeBytes / 4) break;

    uint32_t chunkId = bytecode[offset];
    uint32_t chunkSize = bytecode[offset + 1];

    // Find SHDR chunk to determine shader type
    if (chunkId == FOURCC_SHDR && chunkSize >= 4) {
      const uint32_t* shaderData = &bytecode[offset + 2];
      out.shaderTokens = shaderData;
      out.shaderTokenCount = chunkSize / 4;
      // First token contains the opcode
      uint32_t opcode = shaderData[0] & 0xFF;

      // Opcode 102 = OPHASHPARAMS (SM4)
      // Opcode 64 = DCL_INPUT_PS (SM4)
      // We determine type from the first real opcode
      if (opcode == 102 || opcode == 0) { // NOP or HASHPARAMS
        // Check for vertex shader vs pixel shader
        // Vertex shaders start with DCL_INPUT
        // Pixel shaders start with DCL_INPUT_PS
        for (uint32_t j = 0; j < chunkSize / 4 - 2; j++) {
          uint32_t op = shaderData[j] & 0xFF;
          if (op == 64) { // DCL_INPUT_PS or similar
            out.isPixelShader = true;
            break;
          }
          if (op == 63) { // DCL_INPUT
            out.isVertexShader = true;
            break;
          }
          if (op == 85) { // DCL_GLOBALFLAGS (compute shader indicator)
            out.isComputeShader = true;
            break;
          }
        }
      }
    }
    if (chunkId == FOURCC_RDEF && chunkSize >= 4) {
      out.rdefData = &bytecode[offset + 2];
      out.rdefSize = chunkSize;
    }
  }

  // Default to vertex shader if type not determined
  if (!out.isVertexShader && !out.isPixelShader && !out.isComputeShader) {
    out.isVertexShader = true;
    VKWIND11_LOG_WARN("DXBC: shader type not determined, assuming vertex shader");
  }

  out.valid = true;

  VKWIND11_LOG_INFO("DXBC: %s shader, SM%u.%u",
    out.isVertexShader ? "vertex" : out.isPixelShader ? "pixel" : "compute",
    out.majorVersion, out.minorVersion);

  return true;
}
