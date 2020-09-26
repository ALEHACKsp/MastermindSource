#pragma once
#include "..\Updater\Offsets.h"

namespace Global
{
	FMinimalViewInfo Info;
	float ProjectionMatrix[4][4];

	bool Visible;
	HWND Window;
	Vector2 WndSize;
	Vector2 MouseDelta;
	uintptr_t BaseAddress;
	float CameraFOV;
	Vector3 CameraRotation;
	Vector3 CameraLocation;
	bool Init;
	PVOID Target;
}

D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}

Vector3 GetBoneWithRotation(USkeletalMeshComponent* mesh, int id)
{
	DWORD_PTR bonearray = (DWORD_PTR)ReadPointer(mesh, FortOffsets::StaticMeshComponent::StaticMesh);
	if (!bonearray) bonearray = (DWORD_PTR)ReadPointer(mesh, FortOffsets::StaticMeshComponent::StaticMesh + 0x10);
	if (!bonearray) return Vector3(0, 0, 0);
	
	FTransform ComponentToWorld = mesh->GetComponentToWorld();
	FTransform bone = *(FTransform*)(bonearray + (id * 0x30));
	D3DMATRIX Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}


D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

VOID ToMatrixWithScaleS(float* in, float out[4][4])
{
	auto* rotation = &in[0];
	auto* translation = &in[4];
	auto* scale = &in[8];

	out[3][0] = translation[0];
	out[3][1] = translation[1];
	out[3][2] = translation[2];

	auto x2 = rotation[0] + rotation[0];
	auto y2 = rotation[1] + rotation[1];
	auto z2 = rotation[2] + rotation[2];

	auto xx2 = rotation[0] * x2;
	auto yy2 = rotation[1] * y2;
	auto zz2 = rotation[2] * z2;
	out[0][0] = (1.0f - (yy2 + zz2)) * scale[0];
	out[1][1] = (1.0f - (xx2 + zz2)) * scale[1];
	out[2][2] = (1.0f - (xx2 + yy2)) * scale[2];

	auto yz2 = rotation[1] * z2;
	auto wx2 = rotation[3] * x2;
	out[2][1] = (yz2 - wx2) * scale[2];
	out[1][2] = (yz2 + wx2) * scale[1];

	auto xy2 = rotation[0] * y2;
	auto wz2 = rotation[3] * z2;
	out[1][0] = (xy2 - wz2) * scale[1];
	out[0][1] = (xy2 + wz2) * scale[0];

	auto xz2 = rotation[0] * z2;
	auto wy2 = rotation[3] * y2;
	out[2][0] = (xz2 + wy2) * scale[2];
	out[0][2] = (xz2 - wy2) * scale[0];

	out[0][3] = 0.0f;
	out[1][3] = 0.0f;
	out[2][3] = 0.0f;
	out[3][3] = 1.0f;
}
VOID MultiplyMatricesS(float a[4][4], float b[4][4], float out[4][4])
{
	for (auto r = 0; r < 4; ++r)
	{
		for (auto c = 0; c < 4; ++c)
		{
			auto sum = 0.0f;

			for (auto i = 0; i < 4; ++i)
			{
				sum += a[r][i] * b[i][c];
			}

			out[r][c] = sum;
		}
	}
}
VOID GetBoneLocation(float compMatrix[4][4], PVOID bones, DWORD index, float out[3])
{
	float boneMatrix[4][4];
	ToMatrixWithScaleS((float*)((PBYTE)bones + (index * 0x30)), boneMatrix);

	float result[4][4];
	MultiplyMatricesS(boneMatrix, compMatrix, result);

	out[0] = result[3][0];
	out[1] = result[3][1];
	out[2] = result[3][2];
}



VOID GetViewProjectionMatrix(FSceneViewProjectionData* projectionData, float out[4][4])
{
	auto loc = &projectionData->ViewOrigin;

	float translation[4][4] =
	{
		{ 1.0f, 0.0f, 0.0f, 0.0f, },
		{ 0.0f, 1.0f, 0.0f, 0.0f, },
		{ 0.0f, 0.0f, 1.0f, 0.0f, },
		{ -loc->x, -loc->y, -loc->z, 0.0f, },
	};

	float temp[4][4];
	MultiplyMatricesS(translation, projectionData->ViewRotationMatrix.M, temp);
	MultiplyMatricesS(temp, projectionData->ProjectionMatrix.M, out);
}
BOOLEAN ProjectWorldToScreen(float viewProjection[4][4], float width, float height, float inOutPosition[3])
{
	float res[4] =
	{
		viewProjection[0][0] * inOutPosition[0] + viewProjection[1][0] * inOutPosition[1] + viewProjection[2][0] * inOutPosition[2] + viewProjection[3][0],
		viewProjection[0][1] * inOutPosition[0] + viewProjection[1][1] * inOutPosition[1] + viewProjection[2][1] * inOutPosition[2] + viewProjection[3][1],
		viewProjection[0][2] * inOutPosition[0] + viewProjection[1][2] * inOutPosition[1] + viewProjection[2][2] * inOutPosition[2] + viewProjection[3][2],
		viewProjection[0][3] * inOutPosition[0] + viewProjection[1][3] * inOutPosition[1] + viewProjection[2][3] * inOutPosition[2] + viewProjection[3][3],
	};

	auto r = res[3];
	if (r > 0)
	{
		auto rhw = 1.0f / r;

		inOutPosition[0] = (((res[0] * rhw) / 2.0f) + 0.5f) * width;
		inOutPosition[1] = (0.5f - ((res[1] * rhw) / 2.0f)) * height;
		inOutPosition[2] = r;

		return TRUE;
	}

	return FALSE;
}



BOOLEAN WorldToScreen(float inOutPosition[3])
{
	return ProjectWorldToScreen(Global::ProjectionMatrix, Global::WndSize.x, Global::WndSize.y, inOutPosition);
}