typedef HRESULT(__fastcall* tD3D11Present)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
HRESULT(*PresentOriginal)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) = nullptr;

#define URotationToRadians( FLOAT )		( ( FLOAT ) * ( M_PI_F / 32768.0f ) ) 
#define URotationToDegree( FLOAT )		( ( FLOAT ) * ( 360.0f / 65536.0f ) ) 

#define DegreeToURotation( FLOAT )			( ( FLOAT ) * ( 65536.0f / 360.0f ) )
#define DegreeToRadian( FLOAT )			( ( FLOAT ) * ( M_PI_F / 180.0f ) )

#define RadianToURotation( FLOAT )		( ( FLOAT ) * ( 32768.0f / M_PI_F ) ) 
#define RadianToDegree( FLOAT )			( ( FLOAT ) * ( 180.0f / M_PI_F ) )

Vector2 CalculateRadarCoordinate(Vector3 Location, FLOAT RadarX, FLOAT RadarY)
{
	Vector2 Return;

	FLOAT CosYaw = cosf(URotationToRadians(Global::CameraRotation.y));
	FLOAT SinYaw = sinf(URotationToRadians(Global::CameraRotation.y));

	FLOAT DeltaX = Location.x - Global::CameraLocation.x;
	FLOAT DeltaY = Location.y - Global::CameraLocation.y;

	FLOAT LocationX = (DeltaY * CosYaw - DeltaX * SinYaw) / 50.0f;
	FLOAT LocationY = (DeltaX * CosYaw + DeltaY * SinYaw) / 50.0f;

	FLOAT Distance = Math::Vec2Length(Vector2(LocationX, LocationY));

	if (Distance >= 125.f - 4.f) {
		LocationX /= Distance;
		LocationX *= 125.f - 4.f;

		LocationY /= Distance;
		LocationY *= 125.f - 4.f;
	}

	Return.x = LocationX + RadarX;
	Return.y = -LocationY + RadarY;

	return Return;
}

Vector2 RotatePoint(Vector2 pointToRotate, Vector2 centerPoint, float angle, bool angleInRadians = false)
{
	if (!angleInRadians)
		angle = (float)(angle * (M_PI_F / 180.f));
	float cosTheta = cosf(angle);
	float sinTheta = sinf(angle);
	Vector2 returnVec = Vector2(cosTheta * (pointToRotate.x - centerPoint.x) - sinTheta * (pointToRotate.y - centerPoint.y), sinTheta * (pointToRotate.x - centerPoint.x) + cosTheta * (pointToRotate.y - centerPoint.y));
	returnVec += centerPoint;
	return returnVec;
}

//Present Hook
void __fastcall Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
{
	if (Global::Init) {
		static bool init;
		if (!init) {

			for (int i = 0; i < 254; i++)
			{
				PressedKeys[i] = false;
			}

			for (int i = 0; i < 64; i++)
			{
				SpreadDebug[i] = E("????????????????????????");
				DisabledPointers[i] = true;
			}

			GUI::NextWndProc = (WNDPROC)SetWindowLongPtrA(Global::Window, GWLP_WNDPROC, (LONG_PTR)GUI::WndProc);
			init = true;
		}

		if (Render::NewFrame(swapChain, Global::WndSize))
		{
			//Render::String({ Global::WndSize.x / 2, 100 }, XorString(L"FINALCHEAT.VIP PRIVATE"), true);

			if (ItemESP::Enabled) {

				if (ItemESP::Boats) {
					for (auto actor : boatPawns) {
						auto root = actor->GetRootComp();
						if (root) {
							Vector3 RootPos = root->GetPos();
							Vector2 ItemPos = ProjectWorldToScreen(RootPos);
							Render::String({ ItemPos.x, ItemPos.y }, XorString(L"Motorboat"), 1, Render::Color(202, 202, 202));
						}
					}
				}

				if (ItemESP::Drops) {
					for (auto actor : dropPawns) {
						auto root = actor->GetRootComp();
						if (root) {
							Vector3 RootPos = root->GetPos();
							Vector2 ItemPos = ProjectWorldToScreen(RootPos);
							Render::String({ ItemPos.x, ItemPos.y }, XorString(L"Supply drop"), 1, Render::Color(47, 213, 232));
						}
					}
				}

				if (ItemESP::Llamas) {
					for (auto actor : llamaPawns) {
						auto root = actor->GetRootComp();
						if (root) {
							Vector3 RootPos = root->GetPos();
							Vector2 ItemPos = ProjectWorldToScreen(RootPos);
							Render::String({ ItemPos.x, ItemPos.y }, XorString(L"Llama"), 1, Render::Color(189, 63, 250));
						}
					}
				}

				if (ItemESP::Chests) {
					for (auto actor : chestPawns) {
						auto root = actor->GetRootComp();
						if (root) {
							Vector3 RootPos = root->GetPos();
							Vector2 ItemPos = ProjectWorldToScreen(RootPos);
							Render::String({ ItemPos.x, ItemPos.y }, XorString(L"Chest"), 1, Render::Color(253, 174, 83));
						}
					}
				}

				if (ItemESP::Weapons) {
					for (auto actor : lootPawns) {
						UFortItemDefinition* definitions = (UFortItemDefinition*)ReadPointer(actor, 0x2A8);

						if (!definitions) continue;

						wchar_t* displayname = definitions->DisplayName.Get();

						if (!displayname) continue;

						char LootTier = definitions->GetTier();

						if (!LootTier) continue;

						if (LootTier < ItemESP::Tier) continue;

						Render::Color Color;

						switch (LootTier)
						{
						case 0:
							Color = Render::Color(202, 202, 202);
							break;

						case 1:
							Color = Render::Color(128, 207, 63);
							break;

						case 2:
							Color = Render::Color(47, 213, 232);
							break;

						case 3:
							Color = Render::Color(189, 63, 250);
							break;

						case 4:
							Color = Render::Color(253, 174, 83);
							break;
						}

						auto root = actor->GetRootComp();

						if (!root) continue;

						Vector3 RootPos = root->GetPos();
						Vector2 LootPos = ProjectWorldToScreen(RootPos);

						Render::String({ LootPos.x, LootPos.y }, displayname, 1, Color);
					}
				}
			}

			if (EnemyESP::Enabled || Aimbot::Enabled) {

				if (EnemyESP::Radar) {
					Render::FillCircle(Vector2((int)EnemyESP::RadarX, (int)EnemyESP::RadarY), Render::Color(30, 30, 36, 125), 125);
					Render::Circle(Vector2((int)EnemyESP::RadarX, (int)EnemyESP::RadarY), Render::Color(30, 30, 36), 125, 1);
					Render::FillCircle(Vector2((int)EnemyESP::RadarX, (int)EnemyESP::RadarY), Render::Color(225, 225, 225, 225), 4);
				}

				float closestDistance = FLT_MAX;
				PVOID closestPawn = NULL;

				for (AActor* actor : playerPawns) {
					player* pawn = (player*)actor;

					if (EnemyESP::Radar) {
						auto RootPos = pawn->GetRootComp()->GetPos();
						Vector2 Location2D = CalculateRadarCoordinate(RootPos, (int)EnemyESP::RadarX, (int)EnemyESP::RadarY);
						Location2D = RotatePoint(Location2D, Vector2((int)EnemyESP::RadarX, (int)EnemyESP::RadarY), -Global::CameraRotation.y);
						Render::FillCircle(Location2D, Render::Color(220, 20, 60, 225), 4);
					}

					USkeletalMeshComponent* mesh = pawn->GetMesh();

					if (mesh) {
						auto playerstate = pawn->GetPlayerState();
						if (playerstate) {
							if (Aimbot::Enabled) {
								if ((Aimbot::VisCheck && LineOfSightTo((PVOID)PlayerController, (PVOID)pawn, &Global::CameraLocation)) || !Aimbot::VisCheck) {
									if (Aimbot::AltTargeting) {
										auto localroot = LocalPawn->GetRootComp();
										auto targetroot = pawn->GetRootComp();
										if (localroot && targetroot) {
											auto dist = Math::FastDist3D(localroot->GetPos(), targetroot->GetPos());
											if (dist < closestDistance) {
												closestDistance = dist;
												closestPawn = pawn;
											}
										}
									}
									else {
										Vector2 target = ProjectWorldToScreen(GetBoneWithRotation(mesh, (int)Aimbot::Bone));
										auto dist = Math::FastDist2D(Global::WndSize / 2, target);
										if (dist < Aimbot::Range && dist < closestDistance) {
											closestDistance = dist;
											closestPawn = pawn;
										}
									}
								}
							}

							Vector2 head = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_HEAD));
							Vector2 chest = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_CHEST));
							Vector2 lshoulder = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_L_SHOULDER_2));
							Vector2 rshoulder = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_R_SHOULDER));
							Vector2 lelbow = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_L_ELBOW));
							Vector2 relbow = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_R_ELBOW));
							Vector2 lhand = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_L_HAND_ROOT_1));
							Vector2 rhand = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_R_HAND_ROOT_1));
							Vector2 pelvis = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_PELVIS_1));
							Vector2 lleg = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_L_LEG_ROOT));
							Vector2 rleg = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_R_LEG_ROOT));
							Vector2 lknee = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_L_KNEE));
							Vector2 rknee = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_R_KNEE));
							Vector2 lfoot = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_L_FOOT_MID));
							Vector2 rfoot = ProjectWorldToScreen(GetBoneWithRotation(mesh, eBone::BONE_R_FOOT_MID));

							std::list < Vector2 > bones = { head, chest, lshoulder, rshoulder, lelbow, relbow, lhand, rhand, pelvis, lleg, rleg, lknee, rknee, lfoot, rfoot };

							auto x0 = chest.x, x1 = chest.x, y0 = chest.y, y1 = chest.y;

							if (EnemyESP::Box || EnemyESP::Skeleton) {
								for (auto a : bones) {
									x0 = min(x0, a.x);
									x1 = max(x1, a.x);

									y0 = min(y0, a.y);
									y1 = max(y1, a.y);
								}

								x0 = std::int32_t(std::round(x0));
								y0 = std::int32_t(std::round(y0));

								x1 = std::int32_t(std::round(x1));
								y1 = std::int32_t(std::round(y1));

								x0 -= (x1 - x0) * 0.45;
								y0 -= (y1 - y0) * 0.15;

								x1 += (x1 - x0) * 0.45;
								y1 += (y1 - y0) * 0.15;
							}

							if (EnemyESP::Box) {

								//Render::Rectangle(Vector2(x0, y0), Vector2(x1 - x0, y1 - y0), Render::Color(0, 0, 0, 50), 3);
								//Render::Rectangle(Vector2(x0, y0), Vector2(x1 - x0, y1 - y0), Render::Color(255, 0, 0, 255), 1);

								Render::FillRectangle(Vector2(x0, y0), Vector2(x1 - x0, y1 - y0), Render::Color(0, 0, 0, 25));
								Render::CorneredRect(x0, y0, x1 - x0, y1 - y0, Render::Color(0, 0, 0, 225), 1);
								Render::CorneredRect(x0 + 1, y0 + 1, x1 - x0 - 2, y1 - y0 - 2, Render::Color(225, 0, 0, 225), 1);
							}

							if (EnemyESP::Skeleton) {

								Render::Line(Vector2(head.x, head.y), Vector2(chest.x, chest.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(chest.x, chest.y), Vector2(lshoulder.x, lshoulder.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(lshoulder.x, lshoulder.y), Vector2(lelbow.x, lelbow.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(lelbow.x, lelbow.y), Vector2(lhand.x, lhand.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(chest.x, chest.y), Vector2(rshoulder.x, rshoulder.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(rshoulder.x, rshoulder.y), Vector2(relbow.x, relbow.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(relbow.x, relbow.y), Vector2(rhand.x, rhand.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(chest.x, chest.y), Vector2(pelvis.x, pelvis.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(pelvis.x, pelvis.y), Vector2(lleg.x, lleg.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(lleg.x, lleg.y), Vector2(lknee.x, lknee.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(lknee.x, lknee.y), Vector2(lfoot.x, lfoot.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(pelvis.x, pelvis.y), Vector2(rleg.x, rleg.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(rleg.x, rleg.y), Vector2(rknee.x, rknee.y), Render::Color(255, 255, 255), 1);
								Render::Line(Vector2(rknee.x, rknee.y), Vector2(rfoot.x, rfoot.y), Render::Color(255, 255, 255), 1);
							}

							if (EnemyESP::Distance) {
								auto localroot = LocalPawn->GetRootComp();
								auto targetroot = pawn->GetRootComp();

								if (localroot && targetroot) {
									std::wstring distance = E(L"[ "); distance += std::to_wstring((int)Math::GameDist(localroot->GetPos(), targetroot->GetPos())); distance += E(L" meters"); distance += E(L" ]");
									Render::String(Vector2((x0 + x1) / 2, y1 + 4), distance.c_str(), 1);
								}
							}

							if (EnemyESP::Name) {
								if (playerstate) {
									if (pawn->GetActorID() != LocalPawn->GetActorID()) { 
										Render::String(Vector2((x0 + x1) / 2, y0 - 16), XorString(L"BOT/AI"), 1);
									}
									else {
										FString playerName;
										SpoofCall<PE>(ProcessEvent, playerstate, GetPlayerName, &playerName, 0);
										if (playerName.c_str()) {
											Render::String(Vector2((x0 + x1) / 2, y0 - 16), playerName.c_str(), 1);
											Free(playerName.c_str());
										}
									}
								}
							}

						}
					}
				}

				if (Aimbot::Enabled) {

					Render::Circle(Global::WndSize / 2, Render::Color(30, 30, 36), Aimbot::Range, 1);

					if (PressedKeys[(int)Aimbot::Key]) {
						Global::Target = closestPawn;
					}
					else {
						Global::Target = NULL;
					}
				}

				looped = false;
			}

			if (Debug::WeaponMenu && !GUI::Visible)
			{
				GUI::Begin(XorString(L"Weapon Stats"), Vector2(50.f, 50.f), { 500.f, 700.f }, { 30.f / 255.f,  30.f / 255.f, 36.f / 255.f, 1.f });

				for (int i = 0; i < 64; i++)
				{
					if (SpreadDebug[i] == E("????????????????????????"))
						continue;

					GUI::CheckBox(std::wstring(SpreadDebug[i].begin(), SpreadDebug[i].end()).c_str(), DisabledPointers[i], XorString(L"..."));
				}

				GUI::End();

				Render::FillCircle(Vector2(GUI::IO.MousePos.x, GUI::IO.MousePos.y), Render::Color(255, 255, 255), 4);
			}

			if (GUI::Visible)
			{
				GUI::Begin(XorString(L"Main"), Vector2(Global::WndSize.x / 2 - 250.f, Global::WndSize.y / 2 - 300.f), { 500.f, 700.f }, { 30.f / 255.f,  30.f / 255.f, 36.f / 255.f, 1.f });

				if (GUI::tab == 0) {
					GUI::Spacing(XorString(L"AIMBOT"));
					GUI::CheckBox(XorString(L"Enabled"), Aimbot::Enabled, XorString(L"..."));
					GUI::CheckBox(XorString(L"Visible check"), Aimbot::VisCheck, XorString(L"..."));
					GUI::CheckBox(XorString(L"360° targeting"), Aimbot::AltTargeting, XorString(L"..."));
					GUI::KeyBind(XorString(L"Aimlock key"), &Aimbot::Key, XorString(L"..."));
					if (!Aimbot::AltTargeting) { GUI::SliderFloat(XorString(L"Range"), &Aimbot::Range, 0.f, 1080.f, XorString(L"...")); }
					GUI::SliderInt(XorString(L"Bone"), &Aimbot::Bone, 0.f, 87.f, XorString(L"..."));
					GUI::CheckBox(XorString(L"Spectator spinner mode"), Aimbot::Meme, XorString(L"..."));
				}

				if (GUI::tab == 1) {
					GUI::Spacing(XorString(L"LOOT VISUAL"));
					GUI::CheckBox(XorString(L"Enabled"), ItemESP::Enabled, XorString(L"..."));
					GUI::CheckBox(XorString(L"Boats"), ItemESP::Boats, XorString(L"..."));
					GUI::CheckBox(XorString(L"Chests"), ItemESP::Chests, XorString(L"..."));
					GUI::CheckBox(XorString(L"Drops"), ItemESP::Drops, XorString(L"..."));
					GUI::CheckBox(XorString(L"Llamas"), ItemESP::Llamas, XorString(L"..."));
					GUI::CheckBox(XorString(L"Weapons"), ItemESP::Weapons, XorString(L"..."));
					GUI::SliderInt(XorString(L"Minimum tier (Weapons)"), &ItemESP::Tier, 0.f, 4.f, XorString(L"..."));

					GUI::Spacing(XorString(L"ENEMY VISUAL"));
					GUI::CheckBox(XorString(L"Enabled"), EnemyESP::Enabled, XorString(L"..."));
					GUI::CheckBox(XorString(L"Name"), EnemyESP::Name, XorString(L"..."));
					GUI::CheckBox(XorString(L"Skeleton"), EnemyESP::Skeleton, XorString(L"..."));
					GUI::CheckBox(XorString(L"Bounding box"), EnemyESP::Box, XorString(L"..."));
					GUI::CheckBox(XorString(L"Distance counter"), EnemyESP::Distance, XorString(L"..."));

					GUI::Spacing(XorString(L"2D RADAR"));
					GUI::CheckBox(XorString(L"Enabled"), EnemyESP::Radar, XorString(L"Enables the radar module"));
					GUI::SliderFloat(XorString(L"Position X"), &EnemyESP::RadarX, 2.f, Global::WndSize.x, XorString(L"The position (in pixels) on your screen"));
					GUI::SliderFloat(XorString(L"Position Y"), &EnemyESP::RadarY, 2.f, Global::WndSize.y, XorString(L"The position (in pixels) on your screen"));

				}

				if (GUI::tab == 2) {
					GUI::Spacing(XorString(L"DEBUG"));
					GUI::CheckBox(XorString(L"Weapon stats menu"), Debug::WeaponMenu, XorString(L"..."));
					GUI::CheckBox(XorString(L"Halt events"), Debug::PE2, XorString(L"..."));

					GUI::Spacing(XorString(L"MISC EXPLOIT"));
					GUI::CheckBox(XorString(L"Airstuck"), Exploits::Airstuck, XorString(L"..."));
					GUI::KeyBind(XorString(L"Stuck key"), &Exploits::Stuckkey, XorString(L"..."));
					GUI::CheckBox(XorString(L"Speedhack"), Exploits::Speedhack, XorString(L"..."));
					GUI::KeyBind(XorString(L"Speed key"), &Exploits::Speedkey, XorString(L"..."));
					GUI::SliderFloat(XorString(L"Player speed"), &Exploits::Speedmult1, 0.1f, 10.f, XorString(L"..."));
					GUI::SliderFloat(XorString(L"Game speed"), &Exploits::Speedmult2, 0.1f, 10.f, XorString(L"..."));

					GUI::Spacing(XorString(L"WEAPON EXPLOIT"));
					GUI::CheckBox(XorString(L"Remove recoil"), Exploits::NoRecoil, XorString(L"..."));
					GUI::CheckBox(XorString(L"Anti-Overheat"), Exploits::NoOverheat, XorString(L"..."));
					GUI::CheckBox(XorString(L"Semi No-Spread"), Exploits::NoSpread2, XorString(L"..."));
					GUI::CheckBox(XorString(L"Full No-Spread"), Exploits::NoSpread, XorString(L"..."));
					GUI::KeyBind(XorString(L"Spread-lock key"), &Exploits::SpreadKey, XorString(L"..."));
					GUI::CheckBox(XorString(L"Rapid fire"), Exploits::RapidFire, XorString(L"..."));
					GUI::SliderFloat(XorString(L"RF mod"), &Exploits::RFMod, 1.f, 10.f, XorString(L"..."));

					GUI::Spacing(XorString(L"FUN"));
					GUI::CheckBox(XorString(L"Anti-aim"), Spinbot::Enabled, XorString(L"Makes you spin fast as fuck in the hopes that others can't hit you"));
					GUI::SliderFloat(XorString(L"Yaw offset"), &Spinbot::YawOff, -360.f, 360.f, XorString(L" "));
					GUI::SliderFloat(XorString(L"Pitch offset"), &Spinbot::PitchOff, -360.f, 360.f, XorString(L" "));
					GUI::SliderFloat(XorString(L"Roll offset"), &Spinbot::RollOff, -360.f, 360.f, XorString(L" "));
					GUI::SliderFloat(XorString(L"Yaw speed"), &Spinbot::Yaw, -360.f, 360.f, XorString(L" "));
					GUI::SliderFloat(XorString(L"Pitch speed"), &Spinbot::Pitch, -360.f, 360.f, XorString(L" "));
					GUI::SliderFloat(XorString(L"Roll speed"), &Spinbot::Roll, -360.f, 360.f, XorString(L" "));
				}

				GUI::End();

				Render::FillCircle(Vector2(GUI::IO.MousePos.x, GUI::IO.MousePos.y), Render::Color(255, 255, 255), 4);
			}

			GUI::IO.MouseDelta = GUI::IO.MousePos - GUI::IO.OldMousePos;
			GUI::IO.OldMousePos = GUI::IO.MousePos;

			Render::EndFrame(swapChain);
		}
	}

	PresentOriginal(swapChain, syncInterval, flags);
}

//HRESULT __fastcall Resize(IDXGISwapChain* Swap, UINT a2, UINT a3, UINT a4, DXGI_FORMAT a5, UINT a6) {
//
//	auto state = oResize(Swap, a2, a3, a4, a5, a6);
//	GUI::Render.Release(); 
//	return state;
//}

//void HookD3D()
//{
//	/*DX11PresentFn* pOBS_Present = (DX11PresentFn*)RVA(FindPattern(XorString("FF 15 ? ? ? ? 48 8B 16 48 8B CE FF 52 10 48"), XorString(L"graphics-hook64.dll")), 6);
//	oPresent = *pOBS_Present; *pOBS_Present = Present;*/
//
//	DX11PresentFn* pOBS_Present = (DX11PresentFn*)(GetModuleBase(XorString(L"IGO64")) + 0x163CE8); 
//	oPresent = *pOBS_Present; *pOBS_Present = Present;
//}