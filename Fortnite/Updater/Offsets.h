#pragma once

UWorld* GWorld;
GObjects* objects;
BOOL(*LineOfSightToInternal)(PVOID PlayerController, PVOID Actor, Vector3* ViewPoint);
FString(*GetObjectNameInternal)(PVOID);
FString(*GetNameByIndexInternal)(PVOID);
VOID(*FreeInternal)(PVOID);

DWORD_PTR UWorldAddr, LineOfSightToActorAddr, FnFreeAddr, GetObjectNameAddr, GetObjectNameByIndexAddr, UObjectArrayAddr;


PVOID GetPlayerName;
PVOID IsInVehicle;
PVOID ClientSetRotation;
PVOID AddYawInput;
PVOID AddPitchInput;
//PVOID K2_TeleportTo;

#include "../hooks/Discord.h"
#include "../hooks/Helper.h"
#include <cstdint>


VOID Free(PVOID buffer)
{
	SpoofCall<VOID>(FreeInternal, buffer);
}
std::wstring GetObjectFirstName(UObject* object)
{
	auto internalName = GetObjectNameInternal(object);

	if (!internalName.c_str())
	{
		return L"";
	}

	std::wstring name(internalName.c_str());
	Free(internalName.c_str());

	return name;
}
std::wstring GetObjectName(UObject* object)
{

	std::wstring name(L"");
	for (auto i = 0; object; object = object->Outer, ++i)
	{
		auto internalName = GetObjectNameInternal(object);
		if (!internalName.c_str())
		{
			break;
		}

		name = internalName.c_str() + std::wstring(i > 0 ? L"." : L"") + name;
		Free(internalName.c_str());
	}

	return name;
}
PVOID FindObject(std::wstring arg)
{

	for (auto array : objects->ObjectArray->Objects)
	{
		auto fuObject = array;
		for (auto i = 0; i < 0x10000 && fuObject->Object; ++i, ++fuObject)
		{
			auto object = fuObject->Object;
			if (object->ObjectFlags != 0x41)
			{
				continue;
			}

			if (GetObjectName(object) == arg)
			{
				return object;
			}
		}
	}

	return NULL;
}
BOOLEAN LineOfSightTo(PVOID PlayerController, PVOID Actor, Vector3* ViewPoint)
{
	return SpoofCall<BOOL>(LineOfSightToInternal, PlayerController, Actor, ViewPoint);
}


namespace FortOffsets
{
    enum Main : uint64_t
    {
        UWorld = 0x8C750C0,
    };

    enum World : uint64_t
    {
        PersistentLevel = 0x30,
        OwningGameInstance = 0x190,
        Levels = 0x148,
    };

    enum Level : uint64_t
    {
        AActors = 0x98,
    };

    enum GameInstance : uint64_t
    {
        LocalPlayers = 0x38,
    };

    enum Player : uint64_t
    {
        PlayerController = 0x30,
    };

    enum Controller : uint64_t
    {
        ControlRotation = 0x288,
    };

    enum PlayerController : uint64_t
    {
        PlayerCameraManager = 0x2B8,
        AcknowledgedPawn = 0x2A0,
    };

    enum Pawn : uint64_t
    {
        PlayerState = 0x240,
    };

    enum Actor : uint64_t
    {
        RootComponent = 0x130,
        CustomTimeDilation = 0x98,
    };

    enum Character : uint64_t
    {
        Mesh = 0x280 + 0x10,
    };

    enum SceneComponent : uint64_t
    {
        RelativeLocation = 0x11C,
        ComponentVelocity = 0x140,
    };

    enum StaticMeshComponent : uint64_t
    {
        ComponentToWorld = 0x1C0,
        StaticMesh = 0x490,
    };

    enum SkinnedMeshComponent : uint64_t
    {
        CachedWorldSpaceBounds = 0x600,
    };

    enum FortPawn : uint64_t
    {
        bIsDBNO = 0x552,
        bIsDying = 0x538,
        CurrentWeapon = 0x5A0,
    };

    enum FortPickup : uint64_t
    {
        PrimaryPickupItemEntry = 0x298,
    };

    enum FortItemEntry : uint64_t
    {
        ItemDefinition = 0x18,
    };

    enum FortItemDefinition : uint64_t
    {
        DisplayName = 0x70,
        Tier = 0x54,
    };

    enum FortPlayerStateAthena : uint64_t
    {
        TeamIndex = 0xE68,
        SquadId = 0xFE4,
    };

    enum FortWeapon : uint64_t
    {
        WeaponData = 0x378,
        AmmoCount = 0x934,
    };

    enum FortWeaponItemDefinition : uint64_t
    {
        WeaponStatHandle = 0x7B8,
    };

    enum FortProjectileAthena : uint64_t
    {
        FireStartLoc = 0x880,
    };

    enum Weapon : uint64_t
    {
        LastFireAbilityTime = 0xA98,
    };

    enum FortBaseWeaponStats : uint64_t
    {
        ReloadTime = 0xFC,
    };

    enum BuildingContainer : uint64_t
    {
        bAlreadySearched = 0xC71,
    };

    enum BuildingWeakSpot : uint64_t
    {
        bActive = 0x238,
    };
}


void InitializeOffsets()
{
	auto addr = (uint8_t*)Helper::PatternScan((uintptr_t)GetModuleHandleA(nullptr), XorString("48 8B 1D ? ? ? ? 48 85 DB 74 3B 41"));
	GWorld = reinterpret_cast<decltype(GWorld)>(RVA(addr, 7));

	addr = (uint8_t*)Helper::PatternScan((uintptr_t)GetModuleHandleA(nullptr), XorString("48 8B 05 ? ? ? ? 4C 8D 3C CD"));
	objects = reinterpret_cast<decltype(objects)>(RVA(addr, 7));

	addr = (uint8_t*)Helper::PatternScan((uintptr_t)GetModuleHandleA(nullptr), XorString("40 53 48 83 EC 20 48 8B D9 48 85 D2 75 45 33 C0 48 89 01 48 89 41 08 8D 50 05 E8 ? ? ? ? 8B 53 08 8D 42 05 89 43 08 3B 43 0C 7E 08 48 8B CB E8 ? ? ? ? 48 8B 0B 48 8D 15 ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 48 8B C3 48 83 C4 20 5B C3 48 8B 42 18"));
	GetObjectNameInternal = reinterpret_cast<decltype(GetObjectNameInternal)>(addr);

	addr = (uint8_t*)Helper::PatternScan((uintptr_t)GetModuleHandleA(nullptr), XorString("48 89 5C 24 ? 55 56 57 48 8B EC 48 83 EC 30 8B"));
	GetNameByIndexInternal = reinterpret_cast<decltype(GetNameByIndexInternal)>(addr);

	addr = (uint8_t*)Helper::PatternScan((uintptr_t)GetModuleHandleA(nullptr), XorString("48 85 C9 74 2E 53 48 83 EC 20 48 8B D9 48 8B 0D ? ? ? ? 48 85 C9 75 0C"));
	FreeInternal = reinterpret_cast<decltype(FreeInternal)>(addr);

	addr = (uint8_t*)Helper::PatternScan((uintptr_t)GetModuleHandleA(nullptr), XorString("40 55 53 56 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 E0 49"));
	LineOfSightToInternal = reinterpret_cast<decltype(LineOfSightToInternal)>(addr);



	GetPlayerName = FindObject(XorString(L"/Script/Engine.PlayerState.GetPlayerName"));
    ClientSetRotation = FindObject(XorString(L"/Script/Engine.Controller.ClientSetRotation"));
	AddYawInput = FindObject(XorString(L"/Script/Engine.PlayerController.AddYawInput"));
	AddPitchInput = FindObject(XorString(L"/Script/Engine.PlayerController.AddPitchInput"));
	IsInVehicle = FindObject(XorString(L"/Script/FortniteGame.FortPlayerPawn.IsInVehicle"));
	//K2_TeleportTo = FindObject(XorString(L"/Script/Engine.Actor.K2_TeleportTo"));
}

