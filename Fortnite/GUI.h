#pragma once
#define reseller_name XorString("MASTERMIND")



const char* const KeyNames[] = {
	"Unknown",
	"VK_LBUTTON",
	"VK_RBUTTON",
	"VK_CANCEL",
	"VK_MBUTTON",
	"VK_XBUTTON1",
	"VK_XBUTTON2",
	"Unknown",
	"VK_BACK",
	"VK_TAB",
	"Unknown",
	"Unknown",
	"VK_CLEAR",
	"VK_RETURN",
	"Unknown",
	"Unknown",
	"VK_SHIFT",
	"VK_CONTROL",
	"VK_MENU",
	"VK_PAUSE",
	"VK_CAPITAL",
	"VK_KANA",
	"Unknown",
	"VK_JUNJA",
	"VK_FINAL",
	"VK_KANJI",
	"Unknown",
	"VK_ESCAPE",
	"VK_CONVERT",
	"VK_NONCONVERT",
	"VK_ACCEPT",
	"VK_MODECHANGE",
	"VK_SPACE",
	"VK_PRIOR",
	"VK_NEXT",
	"VK_END",
	"VK_HOME",
	"VK_LEFT",
	"VK_UP",
	"VK_RIGHT",
	"VK_DOWN",
	"VK_SELECT",
	"VK_PRINT",
	"VK_EXECUTE",
	"VK_SNAPSHOT",
	"VK_INSERT",
	"VK_DELETE",
	"VK_HELP",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"VK_LWIN",
	"VK_RWIN",
	"VK_APPS",
	"Unknown",
	"VK_SLEEP",
	"VK_NUMPAD0",
	"VK_NUMPAD1",
	"VK_NUMPAD2",
	"VK_NUMPAD3",
	"VK_NUMPAD4",
	"VK_NUMPAD5",
	"VK_NUMPAD6",
	"VK_NUMPAD7",
	"VK_NUMPAD8",
	"VK_NUMPAD9",
	"VK_MULTIPLY",
	"VK_ADD",
	"VK_SEPARATOR",
	"VK_SUBTRACT",
	"VK_DECIMAL",
	"VK_DIVIDE",
	"VK_F1",
	"VK_F2",
	"VK_F3",
	"VK_F4",
	"VK_F5",
	"VK_F6",
	"VK_F7",
	"VK_F8",
	"VK_F9",
	"VK_F10",
	"VK_F11",
	"VK_F12",
	"VK_F13",
	"VK_F14",
	"VK_F15",
	"VK_F16",
	"VK_F17",
	"VK_F18",
	"VK_F19",
	"VK_F20",
	"VK_F21",
	"VK_F22",
	"VK_F23",
	"VK_F24",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"VK_NUMLOCK",
	"VK_SCROLL",
	"VK_OEM_NEC_EQUAL",
	"VK_OEM_FJ_MASSHOU",
	"VK_OEM_FJ_TOUROKU",
	"VK_OEM_FJ_LOYA",
	"VK_OEM_FJ_ROYA",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"VK_LSHIFT",
	"VK_RSHIFT",
	"VK_LCONTROL",
	"VK_RCONTROL",
	"VK_LMENU",
	"VK_RMENU"
};
bool Hotkey(const char* label, int* k)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);

	ImVec2 size = ImVec2(95, 19);
	const ImRect frame_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y), ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));
	const ImRect total_bb(window->DC.CursorPos, frame_bb.Max);

	ImGui::ItemSize(total_bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	const bool focus_requested = ImGui::FocusableItemRegister(window, g.ActiveId == id);

	const bool focus_requested_by_code = focus_requested;
	const bool focus_requested_by_tab = focus_requested && !focus_requested_by_code;

	const bool hovered = ImGui::ItemHoverable(frame_bb, id);

	if (hovered)
	{
		ImGui::SetHoveredID(id);
		g.MouseCursor = ImGuiMouseCursor_TextInput;
	}

	const bool user_clicked = hovered && io.MouseClicked[0];

	if (focus_requested || user_clicked)
	{
		if (g.ActiveId != id)
		{
			// Start edition
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
			memset(io.KeysDown, 0, sizeof(io.KeysDown));
			*k = 0;
		}
		ImGui::SetActiveID(id, window);
		ImGui::FocusWindow(window);
	}
	else if (io.MouseClicked[0])
	{
		if (g.ActiveId == id)
			ImGui::ClearActiveID();
	}

	bool value_changed = false;
	int key = *k;

	if (g.ActiveId == id)
	{
		for (auto i = 0; i < 5; i++)
		{
			if (io.MouseDown[i])
			{
				switch (i)
				{
				case 0:
					key = VK_LBUTTON;
					break;
				case 1:
					key = VK_RBUTTON;
					break;
				case 2:
					key = VK_MBUTTON;
					break;
				case 3:
					key = VK_XBUTTON1;
					break;
				case 4:
					key = VK_XBUTTON2;
					break;
				}
				value_changed = true;
				ImGui::ClearActiveID();
			}
		}
		if (!value_changed)
		{
			for (auto i = VK_BACK; i <= VK_RMENU; i++)
			{
				if (io.KeysDown[i])
				{
					key = i;
					value_changed = true;
					ImGui::ClearActiveID();
				}
			}
		}

		if (PressedKeys[VK_ESCAPE])
		{
			*k = 0;
			ImGui::ClearActiveID();
		}
		else
		{
			*k = key;
		}
	}

	char buf_display[64] = "No Key";

	ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImVec4(0.44f, 0.44f, 0.44f, 0.40f)), true, style.FrameRounding);

	if (*k != 0 && g.ActiveId != id)
	{
		strcpy_s(buf_display, KeyNames[*k]);
	}
	else if (g.ActiveId == id) {
		strcpy_s(buf_display, "Press a key...");
	}

	const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y);
	ImVec2 render_pos = ImVec2(frame_bb.Min.x + style.FramePadding.x, frame_bb.Min.y + style.FramePadding.y);
	ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x + style.FramePadding.x, frame_bb.Min.y + style.FramePadding.y), ImVec2(frame_bb.Max.x - style.FramePadding.x, frame_bb.Max.y - style.FramePadding.y), buf_display, NULL, NULL, style.ButtonTextAlign, &clip_rect);

	ImGui::RenderText(ImVec2(total_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	return value_changed;
}


namespace GayNite_GUI
{

	void DrawMenu()
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImGui::GetIO().MousePos, ImVec2(ImGui::GetIO().MousePos.x + 5.f, ImGui::GetIO().MousePos.y + 5.f), ImColor(255, 0, 0, 255));
		if (ImGui::Begin(XorString("##menu"), &Global::Visible, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::SetWindowPos(ImVec2(10.f, 100.f), ImGuiCond_Once);
			ImGui::SetWindowSize(ImVec2(660.f, 770.f), ImGuiCond_Once);

			std::string TextGUIName = reseller_name;
			TextGUIName += XorString(" | Last Update at: "); 
			TextGUIName += __DATE__;
			TextGUIName += XorString(" ");
			TextGUIName += __TIME__;
			TextGUIName += XorString(" UTC");

			if (Colours::IsWhiteMenu)
				ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), TextGUIName.c_str()); // Black

			if (ImGui::CollapsingHeader(XorString("ESP")))
			{
				ImGui::Columns(2, XorString("##esp_cols"));

				static float col_width = 0.f;
				if (col_width == 0.f)
					col_width = ImGui::GetColumnWidth(1);

				ImGui::SetColumnWidth(1, col_width - 10.f);

				ImGui::Checkbox(XorString("Enabled##esp"), &EnemyESP::Enabled);
				ImGui::SliderInt(XorString("Max Distance##esp"), &EnemyESP::DistanceLimit, 25, 1000, XorString("%.0f"));
				ImGui::Checkbox(XorString("Players Boxes"), &EnemyESP::Box);
				ImGui::Checkbox(XorString("Players Skeleton"), &EnemyESP::Skeleton);
				ImGui::Checkbox(XorString("Players Names"), &EnemyESP::Name);
				ImGui::Checkbox(XorString("Players Distance"), &EnemyESP::Distance);
				ImGui::Checkbox(XorString("Players Chams"), &EnemyESP::Chams);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(XorString("Disabling Skeleton ESP and Name ESP while this is enabled is recommended.\nIf you disable this, it will not disable till you have started a new game."));

				ImGui::NextColumn();

				ImGui::ColorPicker3(XorString("Player Bones"), (float*)&Colours::SkeletonESPColour, ImGuiColorEditFlags_NoOptions);
			}

			ImGui::Columns(1, XorString("##restore_cols_1"));

			if (ImGui::CollapsingHeader(XorString("Items & Objects")))
			{
				ImGui::Checkbox(XorString("Enabled"), &ItemESP::Enabled);
				ImGui::Checkbox(XorString("Items"), &ItemESP::Weapons);
				ImGui::Checkbox(XorString("Chests"), &ItemESP::Chests);
				//ImGui::Checkbox(XorString("Ammos"), &ItemESP::Ammo);
				//ImGui::Checkbox(XorString("Materials"), &ItemESP::Materials);
				ImGui::Checkbox(XorString("Boats"), &ItemESP::Boats);
				ImGui::Checkbox(XorString("Llamas"), &ItemESP::Llamas);
				ImGui::Checkbox(XorString("Weakspot"), &ItemESP::WeakSpot);

				ImGui::Spacing(); ImGui::Spacing();

				ImGui::Text(XorString("Many more features coming soon!"));
			}

			ImGui::Columns(1, XorString("##restore_cols_1"));

			if (ImGui::CollapsingHeader(XorString("Aimbot & Misc")))
			{
				ImGui::Columns(2, XorString("##aimbot_cols"));

				ImGui::Checkbox(XorString("Enabled##aimbot"), &Aimbot::Enabled);
				ImGui::Checkbox(XorString("Circle##aimbot"), &Aimbot::DrawFov);
				ImGui::Checkbox(XorString("Weakspot Aimbot##aimbot"), &Aimbot::WeakSpot);
				ImGui::Checkbox(XorString("Line to Target##aimbot"), &Aimbot::DrawLine);
				ImGui::SliderInt(XorString("FOV##aimbot"), &Aimbot::Range, 50, 1337, XorString("%.0f"));
				ImGui::SliderInt(XorString("Max Distance##aimbot"), &Aimbot::DistanceLimit, 25, 1000, XorString("%.0f"));
				if (Aimbot::CurrentAimType != "Silent Aimbot")
					ImGui::SliderFloat(XorString("Smoothness##aimbot"), &Aimbot::Smooth, 0.f, 20.f, XorString("%.2f"));
				Hotkey(XorString("Aimbot Keybind##AimKey"), &Aimbot::Key);


				/*ImGui::Spacing(); ImGui::Spacing();
				ImGui::Checkbox(XorString("Boat Fly (BUGGY)"), &Exploits::BoatFly);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(XorString("Boat Fly may crash your game a lot."));

				if (Exploits::BoatFly)
				{
					Hotkey(XorString("Boat Fly Key##flykey"), &Exploits::BoatKey);
					ImGui::SliderFloat(XorString("Boat Fly Speed##flyspeed"), &Exploits::BoatFlySpeed, 0.f, 75.f, XorString("%.1f"));
				}*/

				ImGui::NextColumn();

				if (ImGui::BeginCombo(XorString("Aimbot Type##aimbot"), Aimbot::CurrentAimType))
				{
					for (int n_type_AimType = 0; n_type_AimType < IM_ARRAYSIZE(Aimbot::SelectableAimTypes); n_type_AimType++)
					{
						bool is_selected_type_AimType = (Aimbot::CurrentAimType == Aimbot::SelectableAimTypes[n_type_AimType]);
						if (ImGui::Selectable(Aimbot::SelectableAimTypes[n_type_AimType], is_selected_type_AimType))
						{
							Aimbot::CurrentAimType = Aimbot::SelectableAimTypes[n_type_AimType];
						}
						if (is_selected_type_AimType)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (ImGui::BeginCombo(XorString("Aimbot Bone##aimbot"), Aimbot::CurrentAimBone))
				{
					for (int n_Bone_AimBone = 0; n_Bone_AimBone < IM_ARRAYSIZE(Aimbot::SelectableAimBones); n_Bone_AimBone++)
					{
						bool is_selected_Bone_AimBone = (Aimbot::CurrentAimBone == Aimbot::SelectableAimBones[n_Bone_AimBone]);
						if (ImGui::Selectable(Aimbot::SelectableAimBones[n_Bone_AimBone], is_selected_Bone_AimBone))
						{
							Aimbot::CurrentAimBone = Aimbot::SelectableAimBones[n_Bone_AimBone];
						}
						if (is_selected_Bone_AimBone)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}


				ImGui::Checkbox(XorString("Custom Crosshair"), &GlobalSettings::Crosshair);
				if (GlobalSettings::Crosshair)
				{
					if (ImGui::BeginCombo(XorString("Crosshair Type##aimbot"), GlobalSettings::CurrentCrosshair))
					{
						for (int n_Bone_Crosshair = 0; n_Bone_Crosshair < IM_ARRAYSIZE(GlobalSettings::SelectableCrosshairs); n_Bone_Crosshair++)
						{
							bool is_selected_Bone_Crosshair = (GlobalSettings::CurrentCrosshair == GlobalSettings::SelectableCrosshairs[n_Bone_Crosshair]);
							if (ImGui::Selectable(GlobalSettings::SelectableCrosshairs[n_Bone_Crosshair], is_selected_Bone_Crosshair))
							{
								GlobalSettings::CurrentCrosshair = GlobalSettings::SelectableCrosshairs[n_Bone_Crosshair];
							}
							if (is_selected_Bone_Crosshair)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
				}
				if (GlobalSettings::Crosshair && GlobalSettings::CurrentCrosshair == "Normal Crosshair" || GlobalSettings::CurrentCrosshair == "Tilted Crosshair")
				{
					ImGui::SliderInt(XorString("Crosshair Size"), &GlobalSettings::CrosshairSize, 10, Global::WndSize.y / 2, XorString("%.1f"));
				}

				ImGui::Checkbox(XorString("In Game FOV Changer"), &Exploits::FOVChangerToggle);
				if (Exploits::FOVChangerToggle)
					ImGui::SliderFloat(XorString("FOV Changer"), &Exploits::DesiredInGameFOV, 80, 160, XorString("%.1f"));

				ImGui::Checkbox(XorString("Visibility Check"), &GlobalSettings::VisCheck);
				//ImGui::Checkbox(XorString("In Game Sound Memes"), &Exploits::MemeSounds);
				//ImGui::Checkbox(XorString("Sniper Bullet Teleport"), &Exploits::SniperTP);
				/*ImGui::Checkbox(XorString("Shoot through walls (=>10m)"), &Exploits::BulletTP);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(XorString("If this is enabled, it will only work on players within 10m\nRequires Vischeck to be off."));*/
			}

			ImGui::End();
		}
	}
}
