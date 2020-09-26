#pragma once
#include "Discord.h"
#include "Helper.h"
#include "utils/Structs.h"

INT(*GetViewPoint)(PVOID, FMinimalViewInfo*, BYTE) = nullptr;
VOID(*CalculateProjectionMatrixGivenView)(FMinimalViewInfo* viewInfo, BYTE aspectRatioAxisConstraint, PBYTE viewport, FSceneViewProjectionData* inOutProjectionData) = nullptr;

/*INT GetViewPointHook(PVOID player, FMinimalViewInfo* viewInfo, BYTE stereoPass)
{
	//auto ret = GetViewPoint(player, viewInfo, stereoPass);

	Global::CameraFOV = viewInfo->FOV;
	Global::CameraLocation = viewInfo->Location;
	Global::CameraRotation = viewInfo->Rotation;

	if (GlobalSettings::AllEnable && Exploits::FOVChangerToggle)
	{
		const float UpperFOV = 50.534008f;
		const float LowerFOV = 40.0f;
		auto DesiredFOV = (((180.0f - UpperFOV) / (180.0f - 80.0f)) * (Exploits::DesiredInGameFOV - 80.0f)) + UpperFOV;
		if (Global::CameraFOV > UpperFOV)
		{
			Global::CameraFOV = DesiredFOV;
		}
		else if (Global::CameraFOV > LowerFOV)
		{
			Global::CameraFOV = (((Global::CameraFOV - LowerFOV) / (UpperFOV - LowerFOV)) * (DesiredFOV - LowerFOV)) + LowerFOV;
		}
		viewInfo->FOV = Global::CameraFOV;
	}

	//return ret;
}*/
VOID CalculateProjectionMatrixGivenViewHook(FMinimalViewInfo* viewInfo, BYTE aspectRatioAxisConstraint, PBYTE viewport, FSceneViewProjectionData* inOutProjectionData)
{
	CalculateProjectionMatrixGivenView(viewInfo, aspectRatioAxisConstraint, viewport, inOutProjectionData);

	Global::Info = *viewInfo;

	GetViewProjectionMatrix(inOutProjectionData, Global::ProjectionMatrix);
}

void SetupCamera()
{
	auto addr = (uint8_t*)Helper::PatternScan((uintptr_t)GetModuleHandleA(nullptr), XorString("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B D9 41 8B F0 48 8B 49 30 48 8B FA E8 ? ? ? ? BA ? ? ? ? 48 8B C8"));
	//DISCORD.HookFunction((uintptr_t)addr, (uintptr_t)GetViewPointHook, (uintptr_t)&GetViewPoint);

	addr = (uint8_t*)Helper::PatternScan((uintptr_t)GetModuleHandleA(nullptr), XorString("F3 0F 5F 0D ? ? ? ? 41 8B 41 08"));
	addr -= 0x280;
	DISCORD.HookFunction((uintptr_t)addr, (uintptr_t)CalculateProjectionMatrixGivenViewHook, (uintptr_t)&CalculateProjectionMatrixGivenView);
}
