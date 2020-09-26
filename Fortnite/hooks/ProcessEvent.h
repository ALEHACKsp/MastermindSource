#pragma once
#include "Discord.h"
#include "Helper.h"
#include <random>
#include <imgui.h>

float* (*CalculateShot)(PVOID, PVOID, PVOID) = nullptr;
PVOID(*ProcessEvent)(PVOID, PVOID, PVOID, PVOID) = nullptr;

UWorld* World;
UGameInstance* GameInstance = nullptr;
ULocalPlayer* LocalPlayer = nullptr;
APlayerController* PlayerController = nullptr;
AActor* LocalPawn = nullptr;
USceneComponent* RootComp = nullptr;
ULevel* MainLevel = nullptr;
TArray < class ULevel* > Levels;

std::vector<AActor*> weakspotPawns;
std::vector<AActor*> playerPawns;
std::vector<AActor*> lootPawns;
std::vector<AActor*> chestPawns;
std::vector<AActor*> llamaPawns;
std::vector<AActor*> dropPawns;
std::vector<AActor*> boatPawns;
//std::vector<AActor*> FlyingPawns;

bool disabled;
int tick;
Vector3 cached;
Vector2 MousePos;
Vector2 MouseDelta;
bool looped;

BOOL IsPawnInVehicle()
{
	bool VehicleReturnValue;

	if (MainLevel)
		SpoofCall(ProcessEvent, (UObject*)LocalPawn, IsInVehicle, &VehicleReturnValue, 0);
	else
		VehicleReturnValue = false;

	return VehicleReturnValue;
}
void ComputeLoop()
{
	if (ItemESP::Enabled || EnemyESP::Enabled || Aimbot::Enabled)
	{
		if (looped)
			return;

		auto World = (UWorld*)(*(uintptr_t*)GWorld);
		if (!World)
		{
			weakspotPawns.clear();
			playerPawns.clear();
			lootPawns.clear();
			chestPawns.clear();
			llamaPawns.clear();
			dropPawns.clear();
			boatPawns.clear();
			//FlyingPawns.clear();
			looped = false;
			return;
		}

		GameInstance = World->GetGameInstance();
		if (!GameInstance)
		{
			weakspotPawns.clear();
			playerPawns.clear();
			lootPawns.clear();
			chestPawns.clear();
			llamaPawns.clear();
			dropPawns.clear();
			boatPawns.clear();
			//FlyingPawns.clear();
			looped = false;
			return;
		}

		LocalPlayer = GameInstance->GetLocalPlayer();
		if (!LocalPlayer)
		{
			weakspotPawns.clear();
			playerPawns.clear();
			lootPawns.clear();
			chestPawns.clear();
			llamaPawns.clear();
			dropPawns.clear();
			boatPawns.clear();
			//FlyingPawns.clear();
			looped = false;
			return;
		}

		PlayerController = LocalPlayer->GetController();
		if (!PlayerController)
		{
			weakspotPawns.clear();
			playerPawns.clear();
			lootPawns.clear();
			chestPawns.clear();
			llamaPawns.clear();
			dropPawns.clear();
			boatPawns.clear();
			//FlyingPawns.clear();
			looped = false;
			return;
		}

		LocalPawn = PlayerController->GetPawn();
		if (!LocalPawn)
		{
			weakspotPawns.clear();
			playerPawns.clear();
			lootPawns.clear();
			chestPawns.clear();
			llamaPawns.clear();
			dropPawns.clear();
			boatPawns.clear();
			//FlyingPawns.clear();
			looped = false;
			return;
		}

		RootComp = LocalPawn->GetRootComp();
		if (!RootComp)
		{
			weakspotPawns.clear();
			playerPawns.clear();
			lootPawns.clear();
			chestPawns.clear();
			llamaPawns.clear();
			dropPawns.clear();
			boatPawns.clear();
			//FlyingPawns.clear();
			looped = false;
			return;
		}

		MainLevel = World->GetMainLevel();
		if (!MainLevel)
		{
			weakspotPawns.clear();
			playerPawns.clear();
			lootPawns.clear();
			chestPawns.clear();
			llamaPawns.clear();
			dropPawns.clear();
			boatPawns.clear();
			//FlyingPawns.clear();
			looped = false;
			return;
		}

		Levels = World->Levels;
		std::vector<AActor*> weakspotPawnsTemp;
		std::vector<AActor*> playerPawnsTemp;
		std::vector<AActor*> lootPawnsTemp;
		std::vector<AActor*> chestPawnsTemp;
		std::vector<AActor*> llamaPawnsTemp;
		std::vector<AActor*> dropPawnsTemp;
		std::vector<AActor*> boatPawnsTemp;
		//std::vector<AActor*> FlyingPawnsTemp;

		player* myself = (player*)LocalPawn;
		APlayerState* mypstate = myself->GetPlayerState();

		for (int i = 0; i < Levels.Num(); i++)
		{
			ULevel* level = Levels[i];
			int entityCount = level->entityList.Num();

			for (int i = 0; i < entityCount; i++)
			{
				AActor* actor = level->entityList[i];

				if (!actor)
					continue;

				if (actor == LocalPawn)
					continue;

				std::wstring O1 = GetObjectFirstName((UObject*)actor);
				if (wcsstr(O1.c_str(), XorString(L"WeakSpot")) && Aimbot::Enabled)
				{
					auto root = actor->GetRootComp();
					auto localroot = LocalPawn->GetRootComp();
					Vector3 RootPos = root->GetPos();
					int DistanceToObject = (int)Math::GameDist(localroot->GetPos(), RootPos);

					if (ReadDWORD(actor, FortOffsets::BuildingWeakSpot::bActive) != 0 && DistanceToObject < 5)
						weakspotPawnsTemp.push_back(actor);
				}
				if (wcsstr(O1.c_str(), XorString(L"PlayerPawn")) && (EnemyESP::Enabled || Aimbot::Enabled))
				{
					if (mypstate)
					{
						player* them = (player*)actor;
						APlayerState* theirpstate = them->GetPlayerState();
						if (theirpstate)
						{
							auto MyTeamIndex = ReadDWORD(mypstate, FortOffsets::FortPlayerStateAthena::TeamIndex);
							auto TheirTeamIndex = ReadDWORD(theirpstate, FortOffsets::FortPlayerStateAthena::TeamIndex);

							if (MyTeamIndex == TheirTeamIndex)
							{
								continue;
							}
							playerPawnsTemp.push_back(actor);
						}
					}
				}
				if (wcsstr(O1.c_str(), XorString(L"FortPickupAthena")) && ItemESP::Enabled && ItemESP::Weapons || ItemESP::Ammo || ItemESP::Materials)
				{
					lootPawnsTemp.push_back(actor);
				}
				if (wcsstr(O1.c_str(), XorString(L"Tiered_Chest")) && ItemESP::Enabled && ItemESP::Chests && !((ReadBYTE(actor, FortOffsets::BuildingContainer::bAlreadySearched) >> 7) & 1))
				{
					chestPawnsTemp.push_back(actor);
				}
				if (wcsstr(O1.c_str(), XorString(L"AthenaSupplyDrop_Llama")) && ItemESP::Enabled && ItemESP::Llamas)
				{
					llamaPawnsTemp.push_back(actor);
				}
				if (wcsstr(O1.c_str(), XorString(L"AthenaSupplyDrop_C")) && ItemESP::Enabled && ItemESP::Drops)
				{
					dropPawnsTemp.push_back(actor);
				}
				if (wcsstr(O1.c_str(), XorString(L"MeatballVehicle")) && ItemESP::Enabled && ItemESP::Boats)
				{
					boatPawnsTemp.push_back(actor);
					//FlyingPawnsTemp.push_back(actor);
				}
			}
		}

		if (World && GameInstance && LocalPlayer && PlayerController && LocalPawn && RootComp && ReadBYTE(LocalPawn, FortOffsets::FortPawn::bIsDying) != 24 && ReadBYTE(LocalPawn, FortOffsets::FortPawn::bIsDying) != 28)
		{
			weakspotPawns = weakspotPawnsTemp;
			playerPawns = playerPawnsTemp;
			lootPawns = lootPawnsTemp;
			chestPawns = chestPawnsTemp;
			llamaPawns = llamaPawnsTemp;
			dropPawns = dropPawnsTemp;
			boatPawns = boatPawnsTemp;
			//FlyingPawns = boatPawnsTemp;
			looped = true;
		}
		else
		{
			weakspotPawns.clear();
			playerPawns.clear();
			lootPawns.clear();
			chestPawns.clear();
			llamaPawns.clear();
			dropPawns.clear();
			boatPawns.clear();
			//FlyingPawns.clear();
			looped = false;
			return;
		}
	}
	else
	{
		weakspotPawns.clear();
		playerPawns.clear();
		lootPawns.clear();
		chestPawns.clear();
		llamaPawns.clear();
		dropPawns.clear();
		boatPawns.clear();
		//FlyingPawns.clear();
		looped = false;
		return;
	}
}

void DoAimbot(player* target)
{
	auto mesh = target->GetMesh();
	auto root = target->GetRootComp();
	auto localroot = LocalPawn->GetRootComp();
	Vector3 RootPos = root->GetPos();

	if (mesh)
	{
		std::wstring ActorName = GetObjectFirstName((UObject*)target);

		if (wcsstr(ActorName.c_str(), XorString(L"PlayerPawn")))
		{
			Vector3 AimLocation1 = GetBoneWithRotation(mesh, Aimbot::Bone);
			Vector3 AimLocation2 = Math::CalcAngle(Global::CameraLocation, AimLocation1);

			if (Aimbot::CurrentAimType == "Memory Aim")
			{
				Vector3 FinalLocation;

				if (Aimbot::Smooth <= 0.0f)
				{
					FinalLocation = AimLocation2;
				}
				else
				{
					auto SmootherScale = Aimbot::Smooth + 1.0f;
					Vector3 CamRot = Global::CameraRotation;
					Vector3 Delta = AimLocation2 - Global::CameraRotation;
					Math::ClampAngle(Delta);
					FinalLocation = CamRot + Delta / SmootherScale;
				}
				FinalLocation.z = 0;
				SpoofCall(ProcessEvent, (UObject*)PlayerController, (UObject*)ClientSetRotation, &FinalLocation, 0);
			}
		}
	}

	/*PVOID ProcessEventHook(UObject * object, UObject * func, PVOID params, PVOID result)
	{
		if (object && func)
		{
			auto objectName = GetObjectFirstName(object);
			auto funcName = GetObjectFirstName(func);

			if ((GlobalSettings::AllEnable && Aimbot::Enabled || ItemESP::Enabled || EnemyESP::Enabled) && wcsstr(funcName.c_str(), XorString(L"Tick")))
			{
				ComputeLoop();

				if (Aimbot::CurrentAimBone == "Head")
					Aimbot::Bone = eBone::BONE_HEAD;

				if (Aimbot::CurrentAimBone == "Neck")
					Aimbot::Bone = eBone::BONE_NECK;

				if (Aimbot::CurrentAimBone == "Chest")
					Aimbot::Bone = eBone::BONE_CHEST;

				if (Aimbot::CurrentAimBone == "Torso")
					Aimbot::Bone = eBone::BONE_TORSO;

				if (Aimbot::CurrentAimBone == "Pelvis")
					Aimbot::Bone = eBone::BONE_PELVIS_1;

				if (GlobalSettings::AllEnable && Aimbot::Enabled && !Global::Visible)
				{
					static bool viewfix;
					if (PressedKeys[(int)Aimbot::Key])
					{
						auto target = (player*)Global::Target;
						if (target)
						{
							auto root = target->GetRootComp();
							auto localroot = LocalPawn->GetRootComp();
							Vector3 RootPos = root->GetPos();

							if (!IsPawnInVehicle())
							{
								int DistanceToPlayer = (int)Math::GameDist(localroot->GetPos(), RootPos);
								if (DistanceToPlayer < Aimbot::DistanceLimit)
								{
									DoAimbot(target);
								}
							}
							else
							{
								DoAimbot(target);
							}
						}
					}
				}
			}
		}

		return ProcessEvent(object, func, params, result);
	}*/
}

/*
float* CalculateShotHook(PVOID arg0, PVOID arg1, PVOID arg2)
{
	auto ret = CalculateShot(arg0, arg1, arg2);
	if (GlobalSettings::AllEnable && ret && Aimbot::Enabled && Aimbot::CurrentAimType == "Silent Aimbot" && !Global::Visible)
	{
		auto target = (player*)Global::Target;
		std::wstring ActorName = GetObjectFirstName((UObject*)target);
		if (wcsstr(ActorName.c_str(), XorString(L"PlayerPawn")))
		{
			if (target)
			{
				if (!IsPawnInVehicle())
				{
					auto roots = target->GetRootComp();
					auto localroot = LocalPawn->GetRootComp();
					Vector3 RootPos = roots->GetPos();

					int DistanceToPlayer = (int)Math::GameDist(localroot->GetPos(), RootPos);

					if (DistanceToPlayer < Aimbot::DistanceLimit)
					{
						auto mesh = target->GetMesh();
						if (mesh)
						{
							auto targetroot = target->GetRootComp();
							if (targetroot)
							{
								Vector3 enemy = GetBoneWithRotation(mesh, Aimbot::Bone);

								if (Exploits::BulletTP)
								{
									if (Math::GameDist(RootComp->GetPos(), targetroot->GetPos()) < 12)
									{
										ret[4] = enemy.x;
										ret[5] = enemy.y;
										ret[6] = enemy.z;
									}
								}
								auto length = sqrt(powf(enemy.x - Global::CameraLocation.x, 2) + powf(enemy.y - Global::CameraLocation.y, 2));
								auto x = -atan2f(enemy.z - Global::CameraLocation.z, length);
								auto y = atan2f(enemy.y - Global::CameraLocation.y, enemy.x - Global::CameraLocation.x);

								x /= 2.0f;
								y /= 2.0f;

								ret[0] = -(sinf(x) * sinf(y));
								ret[1] = sinf(x) * cosf(y);
								ret[2] = cosf(x) * sinf(y);
								ret[3] = cosf(x) * cosf(y);
							}
						}
					}
				}
				else
				{
					auto mesh = target->GetMesh();
					if (mesh)
					{
						auto targetroot = target->GetRootComp();
						if (targetroot)
						{
							Vector3 enemy = GetBoneWithRotation(mesh, Aimbot::Bone);

							if (Exploits::BulletTP)
							{
								if (Math::GameDist(RootComp->GetPos(), targetroot->GetPos()) < 12)
								{
									ret[4] = enemy.x;
									ret[5] = enemy.y;
									ret[6] = enemy.z;
								}
							}
							auto length = sqrt(powf(enemy.x - Global::CameraLocation.x, 2) + powf(enemy.y - Global::CameraLocation.y, 2));
							auto x = -atan2f(enemy.z - Global::CameraLocation.z, length);
							auto y = atan2f(enemy.y - Global::CameraLocation.y, enemy.x - Global::CameraLocation.x);

							x /= 2.0f;
							y /= 2.0f;

							ret[0] = -(sinf(x) * sinf(y));
							ret[1] = sinf(x) * cosf(y);
							ret[2] = cosf(x) * sinf(y);
							ret[3] = cosf(x) * cosf(y);
						}
					}
				}
			}
		}
	}

	return ret;
}
*/
