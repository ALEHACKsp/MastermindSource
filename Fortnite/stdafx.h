#pragma once

#include <stdio.h>
#include <windows.h>
#include <winternl.h>
#include <string>
#include <sstream>
#include <vector>
#include <list>

#include <intrin.h>

#pragma intrinsic(_ReturnAddress)

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"


#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "utils/EncStr.h"
#include "utils/Utils.h"
#include "utils/Math.h"
#include "utils/Bones.h"
#include "utils/Structs.h"
#include "utils/MoreUtils.h"
#include "Updater/Offsets.h"

bool PressedKeys[254];

static void Text(const ImVec2& pos, ImColor col, bool outline, const char* fmt, ...)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window)
	{
		va_list args1;
		va_start(args1, fmt);
		va_list args2;
		va_copy(args2, args1);
		std::vector<char> buf(1 + std::vsnprintf(nullptr, 0, fmt, args1));
		va_end(args1);
		std::vsnprintf(buf.data(), buf.size(), fmt, args2);
		va_end(args2);
		if (outline)
		{
			window->DrawList->AddText(ImVec2(pos.x + 1, pos.y + 1), ImColor(0, 0, 0, 255), buf.data());
			window->DrawList->AddText(ImVec2(pos.x - 1, pos.y - 1), ImColor(0, 0, 0, 255), buf.data());
		}
		window->DrawList->AddText(pos, col, buf.data());
	}
}

namespace Spinbot
{
	inline bool Enabled = false;
	inline float YawOff = 0.f;
	inline float PitchOff = 0.f;
	inline float RollOff = 0.f;
	inline float Yaw = 0.f;
	inline float Pitch = 0.f;
	inline float Roll = 0.f;
}
namespace Exploits
{
	bool SniperTP = false;
	bool BulletTP = false;
	float DesiredInGameFOV = 125.0f;
	bool Speedhack = false;
	float Speedmult1 = 2.f;
	float Speedmult2 = 2.f;
	int Speedkey = 88;

	bool Airstuck = false;
	int Stuckkey = 67;

	bool NoOverheat = false;
	bool NoRecoil = false;
	bool NoSpread = false;
	bool NoSpread2 = false;
	bool MemeSounds = false;
	bool FOVChangerToggle = false;
	bool ShootThroughWalls = false;

	int SpreadKey = 90;

	bool RapidFire = false;
	bool FirstPerson = false;

	bool BoatFly = false;
	int BoatKey = 0x10;
	float BoatFlySpeed = 50.f;

	float RFMod = 1.2f;
}
namespace Aimbot
{
	inline bool Enabled = true;
	inline bool WeakSpot = true;
	inline int DistanceLimit = 350;
	inline static const char* SelectableAimTypes[] = { "Memory Aim", "Input Aim"/*, "Mouse Aim", "Silent Aimbot"*/};
	inline static const char* CurrentAimType = "Memory Aim";

	inline static const char* SelectableAimBones[] = { "Head", "Neck", "Chest", "Torso",  "Pelvis" };
	inline static const char* CurrentAimBone = "Chest";



	//bool BulletTP = false;
	inline bool DrawFov = true;
	inline bool DrawLine = false;
	inline bool Meme = false;
	//bool AltTargeting = false;
	inline int Range = 350;
	inline 	float Smooth = 1.5f;
	inline int Bone = 5;
	inline int Key = 0x02;
}
namespace ItemESP
{
	inline bool Enabled = true;
	inline bool Ammo = false;
	inline bool Materials = false;
	inline bool Boats = true;
	inline bool Chests = true;
	inline bool Drops = true;
	inline bool Llamas = true;
	inline bool Weapons = true;
	inline bool WeakSpot = true;
	inline int Tier = 3;
}
namespace EnemyESP
{
	inline bool Radar = false;
	inline bool Chams = false;
	inline int RadarX = 0;
	inline int RadarY = 0;
	inline bool Enabled = true;
	inline bool Name = true;
	inline bool Distance = true;
	inline bool Skeleton = true;
	inline bool Box = true;
	inline int DistanceLimit = 350;
	inline float SkeletonThickness = 1.0f;
}
namespace GlobalSettings
{
	inline bool DebugObjects = false;

	inline int CrosshairSize = 12;
	inline bool AllEnable = true;
	inline bool VisCheck = true;

	inline bool Crosshair = true;
	inline static const char* SelectableCrosshairs[] = { "Normal Crosshair", "Tilted Crosshair", "Cross Style 1", "Cross Style 2" };
	inline static const char* CurrentCrosshair = "Normal Crosshair";
	inline static player* TargetPawn;
}
namespace Colours
{
	inline bool IsWhiteMenu = false;
	inline bool IsBlackMenu = false;

	inline ImVec4 FOVCircleColour = { 255.f / 255.f, 255.f / 255.f, 5.f / 255.f, 1 };				// Yellow
	inline ImVec4 BoundingBoxESPColour = { 255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1 };		// White
	inline ImVec4 BoundingBoxESPColourVisible = { 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 };	// Red
	inline ImVec4 SkeletonESPColour = { 255.f / 255.f, 0.f / 255.f, 255.f / 255.f, 1 };			// Purple
	inline ImVec4 GlobalTextColour = { 255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1 };			// White

	inline ImVec4 White = { 255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1 };			// White
	inline ImVec4 Red = { 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 };			// White
}


ImGuiWindow& BeginScene();
VOID EndScene(ImGuiWindow& window);