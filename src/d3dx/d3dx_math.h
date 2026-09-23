#pragma once

// ============================================================================
// D3DX Math Types & Functions (D3DX11)
// ============================================================================
//
// Stub header for D3DX11 math. Full implementation in later phase.
// This header exists so the build compiles and games that link
// against d3dx11.dll can resolve symbols.
//

#include <cstdint>
#include <cstring>
#include <cmath>

// Basic vector types used by D3DX11 math functions
struct D3DXVECTOR2 { float x, y; };
struct D3DXVECTOR3 {
  float x, y, z;
  D3DXVECTOR3() : x(0), y(0), z(0) {}
  D3DXVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
  D3DXVECTOR3 operator-(const D3DXVECTOR3& r) const { return {x - r.x, y - r.y, z - r.z}; }
  D3DXVECTOR3 operator+(const D3DXVECTOR3& r) const { return {x + r.x, y + r.y, z + r.z}; }
  D3DXVECTOR3 operator*(float s) const { return {x * s, y * s, z * s}; }
  float dot(const D3DXVECTOR3& r) const { return x * r.x + y * r.y + z * r.z; }
  D3DXVECTOR3 cross(const D3DXVECTOR3& r) const {
    return {y * r.z - z * r.y, z * r.x - x * r.z, x * r.y - y * r.x};
  }
  D3DXVECTOR3 normalize() const {
    float len = sqrtf(x * x + y * y + z * z);
    if (len > 0.0f) { float inv = 1.0f / len; return {x * inv, y * inv, z * inv}; }
    return {0, 0, 0};
  }
  float length() const { return sqrtf(x * x + y * y + z * z); }
};
struct D3DXVECTOR4 { float x, y, z, w; };

// 4x4 matrix
struct D3DXMATRIX {
  union {
    struct { float m[4][4]; };
    float v[16];
  };
  D3DXMATRIX() { memset(m, 0, sizeof(m)); }
  explicit D3DXMATRIX(const float* pf) { memcpy(v, pf, 64); }
  float& operator()(int r, int c) { return m[r][c]; }
  const float& operator()(int r, int c) const { return m[r][c]; }
  operator float*() { return v; }
  operator const float*() const { return v; }
};

// Quaternion
struct D3DXQUATERNION {
  float x, y, z, w;
  D3DXQUATERNION() : x(0), y(0), z(0), w(1) {}
  D3DXQUATERNION(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
  void normalize() {
    float len = sqrtf(x * x + y * y + z * z + w * w);
    if (len > 0.0f) { float inv = 1.0f / len; x *= inv; y *= inv; z *= inv; w *= inv; }
  }
};

// Plane
struct D3DXPLANE { float a, b, c, d; };

// Color
struct D3DXCOLOR { float r, g, b, a; D3DXCOLOR() : r(0), g(0), b(0), a(1) {} };

// Matrix operations
D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* pOut);
D3DXMATRIX* D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pIn);
D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* pOut, float* pDeterminant, const D3DXMATRIX* pIn);
D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX* pOut, const D3DXMATRIX* p1, const D3DXMATRIX* p2);
D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX* pOut, float sx, float sy, float sz);
D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX* pOut, float x, float y, float z);
D3DXMATRIX* D3DXMatrixRotationX(D3DXMATRIX* pOut, float Angle);
D3DXMATRIX* D3DXMatrixRotationY(D3DXMATRIX* pOut, float Angle);
D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX* pOut, float Angle);
D3DXMATRIX* D3DXMatrixRotationQuaternion(D3DXMATRIX* pOut, const D3DXQUATERNION* pQ);
D3DXMATRIX* D3DXMatrixRotationAxis(D3DXMATRIX* pOut, const D3DXVECTOR3* pV, float Angle);
D3DXMATRIX* D3DXMatrixRotationYawPitchRoll(D3DXMATRIX* pOut, float Yaw, float Pitch, float Roll);
D3DXMATRIX* D3DXMatrixLookAtLH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye, const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp);
D3DXMATRIX* D3DXMatrixLookAtRH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye, const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp);
D3DXMATRIX* D3DXMatrixPerspectiveFovLH(D3DXMATRIX* pOut, float fovy, float Aspect, float zn, float zf);
D3DXMATRIX* D3DXMatrixPerspectiveFovRH(D3DXMATRIX* pOut, float fovy, float Aspect, float zn, float zf);
float D3DXMatrixDeterminant(const D3DXMATRIX* pM);

// Vector operations
float D3DXVec3Length(const D3DXVECTOR3* pV);
float D3DXVec3Dot(const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2);
D3DXVECTOR3* D3DXVec3Cross(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2);
D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV);
D3DXVECTOR3* D3DXVec3TransformCoord(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM);
D3DXVECTOR3* D3DXVec3TransformNormal(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM);
D3DXVECTOR4* D3DXVec4Transform(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV, const D3DXMATRIX* pM);

// Quaternion operations
D3DXQUATERNION* D3DXQuaternionRotationMatrix(D3DXQUATERNION* pOut, const D3DXMATRIX* pM);
D3DXQUATERNION* D3DXQuaternionRotationAxis(D3DXQUATERNION* pOut, const D3DXVECTOR3* pV, float Angle);
D3DXQUATERNION* D3DXQuaternionRotationYawPitchRoll(D3DXQUATERNION* pOut, float Yaw, float Pitch, float Roll);
D3DXQUATERNION* D3DXQuaternionSlerp(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ1, const D3DXQUATERNION* pQ2, float t);
D3DXQUATERNION* D3DXQuaternionNormalize(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ);

// Plane operations
D3DXPLANE* D3DXPlaneNormalize(D3DXPLANE* pOut, const D3DXPLANE* pP);

// Color operations
D3DXCOLOR* D3DXColorLerp(D3DXCOLOR* pOut, const D3DXCOLOR* pC1, const D3DXCOLOR* pC2, float s);
D3DXCOLOR* D3DXColorAdjustContrast(D3DXCOLOR* pOut, const D3DXCOLOR* pC, float c);
D3DXCOLOR* D3DXColorAdjustSaturation(D3DXCOLOR* pOut, const D3DXCOLOR* pC, float s);
