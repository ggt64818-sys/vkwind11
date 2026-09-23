#include "d3dx_math.h"

// ============================================================================
// D3DX Math Function Stubs
// ============================================================================
//
// Stub implementations. Games that use D3DX math functions will
// link against these. We provide working implementations for common
// functions (matrix inverse, determinant, etc.) and stubs for the rest.
//

// Identity matrix
D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* pOut) {
  if (!pOut) return nullptr;
  memset(pOut->m, 0, sizeof(pOut->m));
  pOut->m[0][0] = pOut->m[1][1] = pOut->m[2][2] = pOut->m[3][3] = 1.0f;
  return pOut;
}

// Matrix transpose
D3DXMATRIX* D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pIn) {
  if (!pOut || !pIn) return nullptr;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      pOut->m[i][j] = pIn->m[j][i];
  return pOut;
}

// Matrix multiply
D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX* pOut, const D3DXMATRIX* p1, const D3DXMATRIX* p2) {
  if (!pOut || !p1 || !p2) return nullptr;
  D3DXMATRIX temp;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      temp.m[i][j] = 0;
      for (int k = 0; k < 4; k++)
        temp.m[i][j] += p1->m[i][k] * p2->m[k][j];
    }
  *pOut = temp;
  return pOut;
}

// Scaling matrix
D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX* pOut, float sx, float sy, float sz) {
  if (!pOut) return nullptr;
  D3DXMatrixIdentity(pOut);
  pOut->m[0][0] = sx;
  pOut->m[1][1] = sy;
  pOut->m[2][2] = sz;
  return pOut;
}

// Translation matrix
D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX* pOut, float x, float y, float z) {
  if (!pOut) return nullptr;
  D3DXMatrixIdentity(pOut);
  pOut->m[3][0] = x;
  pOut->m[3][1] = y;
  pOut->m[3][2] = z;
  return pOut;
}

// Rotation X
D3DXMATRIX* D3DXMatrixRotationX(D3DXMATRIX* pOut, float Angle) {
  if (!pOut) return nullptr;
  float s = sinf(Angle), c = cosf(Angle);
  D3DXMatrixIdentity(pOut);
  pOut->m[1][1] = c;   pOut->m[1][2] = s;
  pOut->m[2][1] = -s;  pOut->m[2][2] = c;
  return pOut;
}

// Rotation Y
D3DXMATRIX* D3DXMatrixRotationY(D3DXMATRIX* pOut, float Angle) {
  if (!pOut) return nullptr;
  float s = sinf(Angle), c = cosf(Angle);
  D3DXMatrixIdentity(pOut);
  pOut->m[0][0] = c;   pOut->m[0][2] = -s;
  pOut->m[2][0] = s;   pOut->m[2][2] = c;
  return pOut;
}

// Rotation Z
D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX* pOut, float Angle) {
  if (!pOut) return nullptr;
  float s = sinf(Angle), c = cosf(Angle);
  D3DXMatrixIdentity(pOut);
  pOut->m[0][0] = c;   pOut->m[0][1] = s;
  pOut->m[1][0] = -s;  pOut->m[1][1] = c;
  return pOut;
}

// Quaternion from axis
D3DXMATRIX* D3DXMatrixRotationAxis(D3DXMATRIX* pOut, const D3DXVECTOR3* pV, float Angle) {
  if (!pOut || !pV) return nullptr;
  D3DXVECTOR3 n = pV->normalize();
  float s = sinf(Angle), c = cosf(Angle);
  float t = 1.0f - c;
  pOut->m[0][0] = t * n.x * n.x + c;       pOut->m[0][1] = t * n.x * n.y + s * n.z; pOut->m[0][2] = t * n.x * n.z - s * n.y; pOut->m[0][3] = 0;
  pOut->m[1][0] = t * n.x * n.y - s * n.z; pOut->m[1][1] = t * n.y * n.y + c;       pOut->m[1][2] = t * n.y * n.z + s * n.x; pOut->m[1][3] = 0;
  pOut->m[2][0] = t * n.x * n.z + s * n.y; pOut->m[2][1] = t * n.y * n.z - s * n.x; pOut->m[2][2] = t * n.z * n.z + c;       pOut->m[2][3] = 0;
  pOut->m[3][0] = 0; pOut->m[3][1] = 0; pOut->m[3][2] = 0; pOut->m[3][3] = 1;
  return pOut;
}

// YawPitchRoll rotation
D3DXMATRIX* D3DXMatrixRotationYawPitchRoll(D3DXMATRIX* pOut, float Yaw, float Pitch, float Roll) {
  if (!pOut) return nullptr;
  D3DXMATRIX mx, my, mz;
  D3DXMatrixRotationX(&mx, Pitch);
  D3DXMatrixRotationY(&my, Yaw);
  D3DXMatrixRotationZ(&mz, Roll);
  D3DXMATRIX temp;
  D3DXMatrixMultiply(&temp, &mz, &mx);
  D3DXMatrixMultiply(pOut, &temp, &my);
  return pOut;
}

// Quaternion from matrix
D3DXQUATERNION* D3DXQuaternionRotationMatrix(D3DXQUATERNION* pOut, const D3DXMATRIX* pM) {
  if (!pOut || !pM) return nullptr;
  float trace = pM->m[0][0] + pM->m[1][1] + pM->m[2][2];
  if (trace > 0) {
    float s = 0.5f / sqrtf(trace + 1.0f);
    pOut->w = 0.25f / s;
    pOut->x = (pM->m[2][1] - pM->m[1][2]) * s;
    pOut->y = (pM->m[0][2] - pM->m[2][0]) * s;
    pOut->z = (pM->m[1][0] - pM->m[0][1]) * s;
  } else if (pM->m[0][0] > pM->m[1][1] && pM->m[0][0] > pM->m[2][2]) {
    float s = 2.0f * sqrtf(1.0f + pM->m[0][0] - pM->m[1][1] - pM->m[2][2]);
    pOut->w = (pM->m[2][1] - pM->m[1][2]) / s;
    pOut->x = 0.25f * s;
    pOut->y = (pM->m[0][1] + pM->m[1][0]) / s;
    pOut->z = (pM->m[0][2] + pM->m[2][0]) / s;
  } else if (pM->m[1][1] > pM->m[2][2]) {
    float s = 2.0f * sqrtf(1.0f + pM->m[1][1] - pM->m[0][0] - pM->m[2][2]);
    pOut->w = (pM->m[0][2] - pM->m[2][0]) / s;
    pOut->x = (pM->m[0][1] + pM->m[1][0]) / s;
    pOut->y = 0.25f * s;
    pOut->z = (pM->m[1][2] + pM->m[2][1]) / s;
  } else {
    float s = 2.0f * sqrtf(1.0f + pM->m[2][2] - pM->m[0][0] - pM->m[1][1]);
    pOut->w = (pM->m[1][0] - pM->m[0][1]) / s;
    pOut->x = (pM->m[0][2] + pM->m[2][0]) / s;
    pOut->y = (pM->m[1][2] + pM->m[2][1]) / s;
    pOut->z = 0.25f * s;
  }
  return pOut;
}

// Quaternion from axis
D3DXQUATERNION* D3DXQuaternionRotationAxis(D3DXQUATERNION* pOut, const D3DXVECTOR3* pV, float Angle) {
  if (!pOut || !pV) return nullptr;
  D3DXVECTOR3 n = pV->normalize();
  float halfAngle = Angle * 0.5f;
  float s = sinf(halfAngle);
  pOut->x = n.x * s;
  pOut->y = n.y * s;
  pOut->z = n.z * s;
  pOut->w = cosf(halfAngle);
  return pOut;
}

// Quaternion YawPitchRoll
D3DXQUATERNION* D3DXQuaternionRotationYawPitchRoll(D3DXQUATERNION* pOut, float Yaw, float Pitch, float Roll) {
  if (!pOut) return nullptr;
  float hy = Yaw * 0.5f, hp = Pitch * 0.5f, hr = Roll * 0.5f;
  float cy = cosf(hy), sy = sinf(hy);
  float cp = cosf(hp), sp = sinf(hp);
  float cr = cosf(hr), sr = sinf(hr);
  pOut->w = cr * cp * cy + sr * sp * sy;
  pOut->x = sr * cp * cy - cr * sp * sy;
  pOut->y = cr * sp * cy + sr * cp * sy;
  pOut->z = cr * cp * sy - sr * sp * cy;
  return pOut;
}

// Quaternion slerp
D3DXQUATERNION* D3DXQuaternionSlerp(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ1, const D3DXQUATERNION* pQ2, float t) {
  if (!pOut || !pQ1 || !pQ2) return nullptr;
  float dot = pQ1->x * pQ2->x + pQ1->y * pQ2->y + pQ1->z * pQ2->z + pQ1->w * pQ2->w;
  D3DXQUATERNION q2 = *pQ2;
  if (dot < 0) { dot = -dot; q2 = {-q2.x, -q2.y, -q2.z, -q2.w}; }
  if (dot > 0.9995f) {
    pOut->x = pQ1->x + t * (q2.x - pQ1->x);
    pOut->y = pQ1->y + t * (q2.y - pQ1->y);
    pOut->z = pQ1->z + t * (q2.z - pQ1->z);
    pOut->w = pQ1->w + t * (q2.w - pQ1->w);
    pOut->normalize();
    return pOut;
  }
  float angle = acosf(dot);
  float sinAngle = sinf(angle);
  float a = sinf((1 - t) * angle) / sinAngle;
  float b = sinf(t * angle) / sinAngle;
  pOut->x = a * pQ1->x + b * q2.x;
  pOut->y = a * pQ1->y + b * q2.y;
  pOut->z = a * pQ1->z + b * q2.z;
  pOut->w = a * pQ1->w + b * q2.w;
  return pOut;
}

// Quaternion normalize
D3DXQUATERNION* D3DXQuaternionNormalize(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ) {
  if (!pOut || !pQ) return nullptr;
  float len = sqrtf(pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w);
  if (len > 0) {
    float inv = 1.0f / len;
    pOut->x = pQ->x * inv;
    pOut->y = pQ->y * inv;
    pOut->z = pQ->z * inv;
    pOut->w = pQ->w * inv;
  } else {
    *pOut = {0, 0, 0, 1};
  }
  return pOut;
}

// Matrix determinant (3x3 sub-determinant for 4x4)
static float det3x3(const D3DXMATRIX& m, int r0, int r1, int r2, int c0, int c1, int c2) {
  return m.m[r0][c0] * (m.m[r1][c1] * m.m[r2][c2] - m.m[r1][c2] * m.m[r2][c1])
       - m.m[r0][c1] * (m.m[r1][c0] * m.m[r2][c2] - m.m[r1][c2] * m.m[r2][c0])
       + m.m[r0][c2] * (m.m[r1][c0] * m.m[r2][c1] - m.m[r1][c1] * m.m[r2][c0]);
}

// Matrix determinant
float D3DXMatrixDeterminant(const D3DXMATRIX* pM) {
  if (!pM) return 0;
  return pM->m[0][0] * det3x3(*pM, 1, 2, 3, 1, 2, 3)
       - pM->m[0][1] * det3x3(*pM, 1, 2, 3, 0, 2, 3)
       + pM->m[0][2] * det3x3(*pM, 1, 2, 3, 0, 1, 3)
       - pM->m[0][3] * det3x3(*pM, 1, 2, 3, 0, 1, 2);
}

// Matrix inverse
D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* pOut, float* pDeterminant, const D3DXMATRIX* pIn) {
  if (!pOut || !pIn) return nullptr;
  float det = D3DXMatrixDeterminant(pIn);
  if (pDeterminant) *pDeterminant = det;
  if (fabsf(det) < 1e-10f) return nullptr;

  float invDet = 1.0f / det;
  D3DXMATRIX out;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      int r0 = (i + 1) % 4, r1 = (i + 2) % 4, r2 = (i + 3) % 4;
      int c0 = (j + 1) % 4, c1 = (j + 2) % 4, c2 = (j + 3) % 4;
      out.m[j][i] = det3x3(*pIn, r0, r1, r2, c0, c1, c2) * invDet;
      if ((i + j) & 1) out.m[j][i] = -out.m[j][i];
    }
  *pOut = out;
  return pOut;
}

// Quaternion to matrix
D3DXMATRIX* D3DXMatrixRotationQuaternion(D3DXMATRIX* pOut, const D3DXQUATERNION* pQ) {
  if (!pOut || !pQ) return nullptr;
  float xx = pQ->x * pQ->x, yy = pQ->y * pQ->y, zz = pQ->z * pQ->z;
  float xy = pQ->x * pQ->y, xz = pQ->x * pQ->z, yz = pQ->y * pQ->z;
  float wx = pQ->w * pQ->x, wy = pQ->w * pQ->y, wz = pQ->w * pQ->z;
  D3DXMatrixIdentity(pOut);
  pOut->m[0][0] = 1 - 2 * (yy + zz); pOut->m[0][1] = 2 * (xy + wz);     pOut->m[0][2] = 2 * (xz - wy);
  pOut->m[1][0] = 2 * (xy - wz);     pOut->m[1][1] = 1 - 2 * (xx + zz); pOut->m[1][2] = 2 * (yz + wx);
  pOut->m[2][0] = 2 * (xz + wy);     pOut->m[2][1] = 2 * (yz - wx);     pOut->m[2][2] = 1 - 2 * (xx + yy);
  return pOut;
}

// LookAt LH
D3DXMATRIX* D3DXMatrixLookAtLH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye, const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp) {
  if (!pOut || !pEye || !pAt || !pUp) return nullptr;
  D3DXVECTOR3 zAxis = (*pAt - *pEye).normalize();
  D3DXVECTOR3 xAxis = pUp->cross(zAxis).normalize();
  D3DXVECTOR3 yAxis = zAxis.cross(xAxis);
  D3DXMatrixIdentity(pOut);
  pOut->m[0][0] = xAxis.x; pOut->m[0][1] = yAxis.x; pOut->m[0][2] = zAxis.x;
  pOut->m[1][0] = xAxis.y; pOut->m[1][1] = yAxis.y; pOut->m[1][2] = zAxis.y;
  pOut->m[2][0] = xAxis.z; pOut->m[2][1] = yAxis.z; pOut->m[2][2] = zAxis.z;
  pOut->m[3][0] = -xAxis.dot(*pEye);
  pOut->m[3][1] = -yAxis.dot(*pEye);
  pOut->m[3][2] = -zAxis.dot(*pEye);
  return pOut;
}

// LookAt RH
D3DXMATRIX* D3DXMatrixLookAtRH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye, const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp) {
  if (!pOut || !pEye || !pAt || !pUp) return nullptr;
  D3DXVECTOR3 zAxis = (*pEye - *pAt).normalize();
  D3DXVECTOR3 xAxis = pUp->cross(zAxis).normalize();
  D3DXVECTOR3 yAxis = zAxis.cross(xAxis);
  D3DXMatrixIdentity(pOut);
  pOut->m[0][0] = xAxis.x; pOut->m[0][1] = yAxis.x; pOut->m[0][2] = zAxis.x;
  pOut->m[1][0] = xAxis.y; pOut->m[1][1] = yAxis.y; pOut->m[1][2] = zAxis.y;
  pOut->m[2][0] = xAxis.z; pOut->m[2][1] = yAxis.z; pOut->m[2][2] = zAxis.z;
  pOut->m[3][0] = -xAxis.dot(*pEye);
  pOut->m[3][1] = -yAxis.dot(*pEye);
  pOut->m[3][2] = -zAxis.dot(*pEye);
  return pOut;
}

// PerspectiveFov LH
D3DXMATRIX* D3DXMatrixPerspectiveFovLH(D3DXMATRIX* pOut, float fovy, float Aspect, float zn, float zf) {
  if (!pOut) return nullptr;
  float yScale = 1.0f / tanf(fovy * 0.5f);
  float xScale = yScale / Aspect;
  memset(pOut->m, 0, sizeof(pOut->m));
  pOut->m[0][0] = xScale;
  pOut->m[1][1] = yScale;
  pOut->m[2][2] = zf / (zf - zn);
  pOut->m[2][3] = 1.0f;
  pOut->m[3][2] = -zn * zf / (zf - zn);
  return pOut;
}

// PerspectiveFov RH
D3DXMATRIX* D3DXMatrixPerspectiveFovRH(D3DXMATRIX* pOut, float fovy, float Aspect, float zn, float zf) {
  if (!pOut) return nullptr;
  float yScale = 1.0f / tanf(fovy * 0.5f);
  float xScale = yScale / Aspect;
  memset(pOut->m, 0, sizeof(pOut->m));
  pOut->m[0][0] = xScale;
  pOut->m[1][1] = yScale;
  pOut->m[2][2] = zf / (zn - zf);
  pOut->m[2][3] = -1.0f;
  pOut->m[3][2] = zn * zf / (zn - zf);
  return pOut;
}

// Vector normalize
D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV) {
  if (!pOut || !pV) return nullptr;
  float len = sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
  if (len > 0) {
    float inv = 1.0f / len;
    pOut->x = pV->x * inv;
    pOut->y = pV->y * inv;
    pOut->z = pV->z * inv;
  } else {
    *pOut = {0, 0, 0};
  }
  return pOut;
}

// Vector dot product
float D3DXVec3Dot(const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) {
  if (!pV1 || !pV2) return 0;
  return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

// Vector cross product
D3DXVECTOR3* D3DXVec3Cross(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) {
  if (!pOut || !pV1 || !pV2) return nullptr;
  *pOut = {
    pV1->y * pV2->z - pV1->z * pV2->y,
    pV1->z * pV2->x - pV1->x * pV2->z,
    pV1->x * pV2->y - pV1->y * pV2->x
  };
  return pOut;
}

// Vector length
float D3DXVec3Length(const D3DXVECTOR3* pV) {
  if (!pV) return 0;
  return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
}

// Vector3 transform coord (w=1)
D3DXVECTOR3* D3DXVec3TransformCoord(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM) {
  if (!pOut || !pV || !pM) return nullptr;
  float w = pM->m[0][3] * pV->x + pM->m[1][3] * pV->y + pM->m[2][3] * pV->z + pM->m[3][3];
  pOut->x = (pM->m[0][0] * pV->x + pM->m[1][0] * pV->y + pM->m[2][0] * pV->z + pM->m[3][0]) / w;
  pOut->y = (pM->m[0][1] * pV->x + pM->m[1][1] * pV->y + pM->m[2][1] * pV->z + pM->m[3][1]) / w;
  pOut->z = (pM->m[0][2] * pV->x + pM->m[1][2] * pV->y + pM->m[2][2] * pV->z + pM->m[3][2]) / w;
  return pOut;
}

// Vector3 transform normal (w=0)
D3DXVECTOR3* D3DXVec3TransformNormal(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM) {
  if (!pOut || !pV || !pM) return nullptr;
  pOut->x = pM->m[0][0] * pV->x + pM->m[1][0] * pV->y + pM->m[2][0] * pV->z;
  pOut->y = pM->m[0][1] * pV->x + pM->m[1][1] * pV->y + pM->m[2][1] * pV->z;
  pOut->z = pM->m[0][2] * pV->x + pM->m[1][2] * pV->y + pM->m[2][2] * pV->z;
  return pOut;
}

// Vector4 transform
D3DXVECTOR4* D3DXVec4Transform(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV, const D3DXMATRIX* pM) {
  if (!pOut || !pV || !pM) return nullptr;
  D3DXVECTOR4 out;
  out.x = pM->m[0][0] * pV->x + pM->m[1][0] * pV->y + pM->m[2][0] * pV->z + pM->m[3][0] * pV->w;
  out.y = pM->m[0][1] * pV->x + pM->m[1][1] * pV->y + pM->m[2][1] * pV->z + pM->m[3][1] * pV->w;
  out.z = pM->m[0][2] * pV->x + pM->m[1][2] * pV->y + pM->m[2][2] * pV->z + pM->m[3][2] * pV->w;
  out.w = pM->m[0][3] * pV->x + pM->m[1][3] * pV->y + pM->m[2][3] * pV->z + pM->m[3][3] * pV->w;
  *pOut = out;
  return pOut;
}

// Plane normalize
D3DXPLANE* D3DXPlaneNormalize(D3DXPLANE* pOut, const D3DXPLANE* pP) {
  if (!pOut || !pP) return nullptr;
  float len = sqrtf(pP->a * pP->a + pP->b * pP->b + pP->c * pP->c);
  if (len > 0) {
    float inv = 1.0f / len;
    pOut->a = pP->a * inv;
    pOut->b = pP->b * inv;
    pOut->c = pP->c * inv;
    pOut->d = pP->d * inv;
  } else {
    *pOut = *pP;
  }
  return pOut;
}

// Color lerp
D3DXCOLOR* D3DXColorLerp(D3DXCOLOR* pOut, const D3DXCOLOR* pC1, const D3DXCOLOR* pC2, float s) {
  if (!pOut || !pC1 || !pC2) return nullptr;
  pOut->r = pC1->r + s * (pC2->r - pC1->r);
  pOut->g = pC1->g + s * (pC2->g - pC1->g);
  pOut->b = pC1->b + s * (pC2->b - pC1->b);
  pOut->a = pC1->a + s * (pC2->a - pC1->a);
  return pOut;
}

// Color adjust contrast
D3DXCOLOR* D3DXColorAdjustContrast(D3DXCOLOR* pOut, const D3DXCOLOR* pC, float c) {
  if (!pOut || !pC) return nullptr;
  pOut->r = 0.5f + c * (pC->r - 0.5f);
  pOut->g = 0.5f + c * (pC->g - 0.5f);
  pOut->b = 0.5f + c * (pC->b - 0.5f);
  pOut->a = pC->a;
  return pOut;
}

// Color adjust saturation
D3DXCOLOR* D3DXColorAdjustSaturation(D3DXCOLOR* pOut, const D3DXCOLOR* pC, float s) {
  if (!pOut || !pC) return nullptr;
  float grey = 0.2125f * pC->r + 0.7154f * pC->g + 0.0721f * pC->b;
  pOut->r = grey + s * (pC->r - grey);
  pOut->g = grey + s * (pC->g - grey);
  pOut->b = grey + s * (pC->b - grey);
  pOut->a = pC->a;
  return pOut;
}
