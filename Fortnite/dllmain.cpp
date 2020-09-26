#include "stdafx.h"
#include "Hooks/ProcessEvent.h"
#include "Hooks/Camera.h"
#include "gui.h"
#include "hooks/Discord.h"
#include "hooks/Helper.h"

ID3D11Device* device = nullptr;
ID3D11DeviceContext* immediateContext = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

WNDPROC WndProcOriginal = nullptr;
HRESULT(*ResizeOriginal)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) = nullptr;
extern IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using f_present = HRESULT(__stdcall*)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
f_present PresentOriginal = nullptr;

float color_red = 1.;
float color_green = 0;
float color_blue = 0;
float color_random = 0.0;

float color_speed = -10.0;
void ColorChange()
{
	static float Color[3];
	static DWORD Tickcount = 0;
	static DWORD Tickcheck = 0;
	ImGui::ColorConvertRGBtoHSV(color_red, color_green, color_blue, Color[0], Color[1], Color[2]);
	if (GetTickCount() - Tickcount >= 1)
	{
		if (Tickcheck != Tickcount)
		{
			Color[0] += 0.001f * color_speed;
			Tickcheck = Tickcount;
		}
		Tickcount = GetTickCount();
	}
	if (Color[0] < 0.0f) Color[0] += 1.0f;
	ImGui::ColorConvertHSVtoRGB(Color[0], Color[1], Color[2], color_red, color_green, color_blue);
}

LRESULT CALLBACK WndProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		// Cheat Needed Keys
	case WM_LBUTTONDOWN:
		PressedKeys[VK_LBUTTON] = true;
		break;
	case WM_LBUTTONUP:
		PressedKeys[VK_LBUTTON] = false;
		break;
	case WM_RBUTTONDOWN:
		PressedKeys[VK_RBUTTON] = true;
		break;
	case WM_RBUTTONUP:
		PressedKeys[VK_RBUTTON] = false;
		break;
	case WM_XBUTTONDOWN:
		PressedKeys[(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 5 : 6] = true;
		break;
	case WM_XBUTTONUP:
		PressedKeys[(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 5 : 6] = false;
		break;
	case WM_KEYDOWN:
		PressedKeys[wParam] = true;
		break;
	case WM_KEYUP:
		if (wParam == VK_INSERT)
		{
			Global::Visible = !Global::Visible;
		}

		// Menu Toggles
		if (wParam == VK_END)
			GlobalSettings::AllEnable = !GlobalSettings::AllEnable;

		if (wParam == VK_PRIOR)
		{
			if (ItemESP::Tier < 5)
			{
				ItemESP::Tier += 1;
			}
		}
		if (wParam == VK_NEXT)
		{
			if (ItemESP::Tier > 0)
			{
				ItemESP::Tier -= 1;
			}
		}

		PressedKeys[wParam] = false;
		break;
	case WM_MOUSEMOVE:
		MousePos.x = (signed short)(lParam);
		MousePos.y = (signed short)(lParam >> 16);
		if (!Global::Visible)
		{
			cached.x -= MouseDelta.y * 0.5f;
			cached.y += MouseDelta.x * 0.5f;
		}
		break;
	}

	if (Global::Visible)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return true;
	}

	return CallWindowProcA(WndProcOriginal, hWnd, uMsg, wParam, lParam);
}

#define URotationToRadians( FLOAT )		( ( FLOAT ) * ( M_PI_F / 32768.0f ) )
#define URotationToDegree( FLOAT )		( ( FLOAT ) * ( 360.0f / 65536.0f ) )

#define DegreeToURotation( FLOAT )		( ( FLOAT ) * ( 65536.0f / 360.0f ) )
#define DegreeToRadian( FLOAT )			( ( FLOAT ) * ( M_PI_F / 180.0f ) )

#define RadianToURotation( FLOAT )		( ( FLOAT ) * ( 32768.0f / M_PI_F ) )
#define RadianToDegree( FLOAT )			( ( FLOAT ) * ( 180.0f / M_PI_F ) )

bool IfFirstStringContains(std::string firstString, std::string secondString)
{
	if (secondString.size() > firstString.size())
		return false;

	for (int i = 0; i < firstString.size(); i++)
	{
		int j = 0;
		if (firstString[i] == secondString[j])
		{
			int k = i;
			while (firstString[i] == secondString[j] && j < secondString.size())
			{
				j++;
				i++;
			}
			if (j == secondString.size())
				return true;
			else
				i = k;
		}
	}
	return false;
}

__declspec(dllexport) HRESULT PresentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
{
	static float width = 0;
	static float height = 0;
	static HWND hWnd = 0;

	if (!device)
	{
		swapChain->GetDevice(__uuidof(device), reinterpret_cast<PVOID*>(&device));
		device->GetImmediateContext(&immediateContext);

		ID3D11Texture2D* renderTarget = nullptr;
		swapChain->GetBuffer(0, __uuidof(renderTarget), reinterpret_cast<PVOID*>(&renderTarget));
		device->CreateRenderTargetView(renderTarget, nullptr, &renderTargetView);
		renderTarget->Release();

		ID3D11Texture2D* backBuffer = 0;
		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (PVOID*)&backBuffer);
		D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
		backBuffer->GetDesc(&backBufferDesc);

		hWnd = FindWindowA(XorString("UnrealWindow"), XorString("Fortnite  "));
		if (!width)
		{
			WndProcOriginal = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook)));
		}

		width = (float)backBufferDesc.Width;
		height = (float)backBufferDesc.Height;
		Global::WndSize = Vector2(width, height);
		backBuffer->Release();

		ImGui_ImplDX11_Init(hWnd, device, immediateContext);
		ImGui_ImplDX11_CreateDeviceObjects();

		Colours::IsWhiteMenu = true;
		ImGui::StyleColorsLight();
	}

	immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
	auto& window = BeginScene();

	if (GlobalSettings::AllEnable && GlobalSettings::Crosshair)
	{
		float ScreenCenterX = Global::WndSize.x / 2;
		float ScreenCenterY = Global::WndSize.y / 2;

		if (GlobalSettings::CurrentCrosshair == "Normal Crosshair")
		{
			window.DrawList->AddLine(ImVec2(ScreenCenterX - GlobalSettings::CrosshairSize, ScreenCenterY), ImVec2((ScreenCenterX - GlobalSettings::CrosshairSize) + (GlobalSettings::CrosshairSize * 2), ScreenCenterY), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 }));
			window.DrawList->AddLine(ImVec2(ScreenCenterX, ScreenCenterY - GlobalSettings::CrosshairSize), ImVec2(ScreenCenterX, (ScreenCenterY - GlobalSettings::CrosshairSize) + (GlobalSettings::CrosshairSize * 2)), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 }));
		}
		if (GlobalSettings::CurrentCrosshair == "Tilted Crosshair")
		{
			window.DrawList->AddLine(ImVec2(ScreenCenterX + GlobalSettings::CrosshairSize, ScreenCenterY + GlobalSettings::CrosshairSize), ImVec2(ScreenCenterX - GlobalSettings::CrosshairSize, ScreenCenterY - GlobalSettings::CrosshairSize), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 }));
			window.DrawList->AddLine(ImVec2(ScreenCenterX - GlobalSettings::CrosshairSize, ScreenCenterY + GlobalSettings::CrosshairSize), ImVec2(ScreenCenterX + GlobalSettings::CrosshairSize, ScreenCenterY - GlobalSettings::CrosshairSize), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 }));
		}
		if (GlobalSettings::CurrentCrosshair == "Cross Style 1")
		{
			window.DrawList->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), 8, ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 }), 8); // Circle
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX - 13, ScreenCenterY), ImVec2((ScreenCenterX - 13) + 10, ScreenCenterY + 1), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 })); // Left Line
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX + 4, ScreenCenterY), ImVec2((ScreenCenterX + 4) + 10, ScreenCenterY + 1), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 })); // Right Line
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX, ScreenCenterY - 13), ImVec2(ScreenCenterX + 1, (ScreenCenterY - 13) + 10), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 })); // Top Line
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX, ScreenCenterY + 4), ImVec2(ScreenCenterX + 1, (ScreenCenterY + 4) + 10), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 })); // Bottom Line
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX - 1, ScreenCenterY - 1), ImVec2((ScreenCenterX - 1) + 1, (ScreenCenterY - 1) + 1), ImGui::GetColorU32({ 55.f / 255.f, 255.f / 255.f, 0.f / 255.f, 1 })); // Dot point
		}
		if (GlobalSettings::CurrentCrosshair == "Cross Style 2")
		{
			window.DrawList->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), 8, ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 }), 8); // Circle
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX - 17, ScreenCenterY), ImVec2((ScreenCenterX - 17) + 10, ScreenCenterY + 1), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 })); // Left Line
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX + 9, ScreenCenterY), ImVec2((ScreenCenterX + 9) + 10, ScreenCenterY + 1), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 })); // Right Line
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX, ScreenCenterY - 17), ImVec2(ScreenCenterX + 1, (ScreenCenterY - 17) + 10), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 })); // Top Line
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX, ScreenCenterY + 9), ImVec2(ScreenCenterX + 1, (ScreenCenterY + 9) + 10), ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 })); // Bottom Line
			window.DrawList->AddRectFilled(ImVec2(ScreenCenterX - 1, ScreenCenterY - 1), ImVec2((ScreenCenterX - 1) + 1, (ScreenCenterY - 1) + 1), ImGui::GetColorU32({ 55.f / 255.f, 255.f / 255.f, 0.f / 255.f, 1 })); // Dot point
		}
	}

	if (GlobalSettings::AllEnable && ItemESP::Enabled)
	{
		if (ItemESP::Boats)
		{
			for (auto actor : boatPawns)
			{
				auto root = actor->GetRootComp();
				if (root)
				{
					auto localroot = LocalPawn->GetRootComp();
					Vector3 RootPos = root->GetPos();
					Vector3 ItemPos = RootPos;
					WorldToScreen(&ItemPos.x);

					std::wstring DisplayName = XorString(L"Boat");
					std::string DistanceTo = std::to_string((int)Math::GameDist(localroot->GetPos(), RootPos)); DistanceTo += std::string(XorString(" m"));
					std::string FullBoatESP;
					if (!IsPawnInVehicle())
						FullBoatESP = std::string(DisplayName.begin(), DisplayName.end()) + XorString(" [") + DistanceTo + XorString("]");
					else
						FullBoatESP = std::string(DisplayName.begin(), DisplayName.end());

					auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullBoatESP.c_str());
					Text(ImVec2(ItemPos.x - size.x / 2, ItemPos.y - size.y / 2), ImGui::GetColorU32({ 0.f / 255.f, 30.f / 255.f, 255.f / 255.f, 1 }), true, FullBoatESP.c_str());
				}
			}
		}

		if (ItemESP::Drops)
		{
			for (auto actor : dropPawns)
			{
				auto root = actor->GetRootComp();
				if (root)
				{
					auto localroot = LocalPawn->GetRootComp();
					Vector3 RootPos = root->GetPos();
					Vector3 ItemPos = RootPos;
					WorldToScreen(&ItemPos.x);

					std::wstring DisplayName = XorString(L"Supply Drop");
					std::string DistanceTo = std::to_string((int)Math::GameDist(localroot->GetPos(), RootPos)); DistanceTo += std::string(XorString(" m"));
					std::string FullSupplyDropESP;
					if (!IsPawnInVehicle())
						FullSupplyDropESP = std::string(DisplayName.begin(), DisplayName.end()) + XorString(" [") + DistanceTo + XorString("]");
					else
						FullSupplyDropESP = std::string(DisplayName.begin(), DisplayName.end());

					auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullSupplyDropESP.c_str());
					Text(ImVec2(ItemPos.x - size.x / 2, ItemPos.y - size.y / 2), ImGui::GetColorU32({ 0.f / 255.f, 26.f / 255.f, 255.f / 255.f, 1 }), true, FullSupplyDropESP.c_str());
				}
			}
		}

		if (ItemESP::Llamas)
		{
			for (auto actor : llamaPawns)
			{
				auto root = actor->GetRootComp();
				if (root)
				{
					auto localroot = LocalPawn->GetRootComp();
					Vector3 RootPos = root->GetPos();
					Vector3 ItemPos = RootPos;
					WorldToScreen(&ItemPos.x);

					std::wstring DisplayName = XorString(L"Llama!");
					std::string DistanceTo = std::to_string((int)Math::GameDist(localroot->GetPos(), RootPos)); DistanceTo += std::string(XorString(" m"));
					std::string FullLlamaESP;
					if (!IsPawnInVehicle())
						FullLlamaESP = std::string(DisplayName.begin(), DisplayName.end()) + XorString(" [") + DistanceTo + XorString("]");
					else
						FullLlamaESP = std::string(DisplayName.begin(), DisplayName.end());

					auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullLlamaESP.c_str());
					Text(ImVec2(ItemPos.x - size.x / 2, ItemPos.y - size.y / 2), ImGui::GetColorU32({ 179.f / 255.f, 0.f / 255.f, 255.f / 255.f, 1 }), true, FullLlamaESP.c_str());
				}
			}
		}

		if (ItemESP::Chests)
		{
			for (auto actor : chestPawns)
			{
				auto root = actor->GetRootComp();
				if (root)
				{
					auto localroot = LocalPawn->GetRootComp();
					Vector3 RootPos = root->GetPos();
					Vector3 ItemPos = RootPos;
					WorldToScreen(&ItemPos.x);

					int DistanceToObject = (int)Math::GameDist(localroot->GetPos(), RootPos);

					std::wstring DisplayName = XorString(L"Chest");
					std::string DistanceTo = std::to_string(DistanceToObject); DistanceTo += std::string(XorString(" m"));
					std::string FullChestESP;
					if (!IsPawnInVehicle())
						FullChestESP = std::string(DisplayName.begin(), DisplayName.end()) + XorString(" [") + DistanceTo + XorString("]");
					else
						FullChestESP = std::string(DisplayName.begin(), DisplayName.end());

					auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullChestESP.c_str());
					Text(ImVec2(ItemPos.x - size.x / 2, ItemPos.y - size.y / 2), ImGui::GetColorU32({ 255.f / 255.f, 255.f / 255.f, 0.f / 255.f, 1 }), true, FullChestESP.c_str());
				}
			}
		}

		if (ItemESP::Weapons)
		{
			for (auto actor : lootPawns)
			{
				UFortItemDefinition* definitions = (UFortItemDefinition*)ReadPointer(actor, FortOffsets::FortPickup::PrimaryPickupItemEntry + FortOffsets::FortItemEntry::ItemDefinition);

				if (!definitions) continue;

				wchar_t* displayname = definitions->DisplayName.Get();

				if (!displayname) continue;

				char LootTier = definitions->GetTier();

				std::wstring DisplayNameAsWString = displayname;
				std::string DisplayNameAsString(DisplayNameAsWString.begin(), DisplayNameAsWString.end());
				if (IfFirstStringContains(DisplayNameAsString, XorString("Ammo: ")))
				{
					continue;
				}
				if (DisplayNameAsString == XorString("Wood"))
				{
					continue;
				}
				if (DisplayNameAsString == XorString("Stone"))
				{
					continue;
				}
				if (DisplayNameAsString == XorString("Metal"))
				{
					continue;
				}

				if (LootTier < ItemESP::Tier) continue;

				ImU32 Color;

				switch (LootTier)
				{
				case 0: // Common - Grey
					Color = ImGui::GetColorU32({ 168.f / 255.f, 168.f / 255.f, 168.f / 255.f, 1 });
					break;
				case 1: // Uncommon - Green
					Color = ImGui::GetColorU32({ 7.f / 255.f, 143.f / 255.f, 4.f / 255.f, 1 });
					break;
				case 2: // Rare - Blue
					Color = ImGui::GetColorU32({ 8.f / 255.f, 1.f / 255.f, 255.f / 255.f, 1 });
					break;
				case 3: // Epic - Purple
					Color = ImGui::GetColorU32({ 112.f / 255.f, 7.f / 255.f, 135.f / 255.f, 1 });
					break;
				case 4: // Legendary - Gold
					Color = ImGui::GetColorU32({ 255.f / 255.f, 200.f / 255.f, 0.f / 255.f, 1 });
					break;
				case 5: // Mythic - Dark Gold
					Color = ImGui::GetColorU32({ 255.f / 255.f, 150.f / 255.f, 0.f / 255.f, 1 });
					break;

				default: // Default - Grey
					Color = ImGui::GetColorU32({ 168.f / 255.f, 168.f / 255.f, 168.f / 255.f, 1 });
					break;
				}

				auto root = actor->GetRootComp();
				if (!root)
					continue;

				auto localroot = LocalPawn->GetRootComp();
				Vector3 RootPos = root->GetPos();
				Vector3 LootPos = RootPos;
				WorldToScreen(&LootPos.x);

				int DistanceToObject = (int)Math::GameDist(localroot->GetPos(), RootPos);

				std::wstring DisplayName = std::wstring(displayname);
				std::string DistanceTo = std::to_string(DistanceToObject); DistanceTo += std::string(XorString(" m"));
				std::string FullLootESP;
				if (!IsPawnInVehicle())
					FullLootESP = std::string(DisplayName.begin(), DisplayName.end()) + XorString(" [") + DistanceTo + XorString("]");
				else
					FullLootESP = std::string(DisplayName.begin(), DisplayName.end());

				auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullLootESP.c_str());
				Text(ImVec2(LootPos.x - size.x / 2, LootPos.y - size.y / 2), Color, true, FullLootESP.c_str());
			}
		}

		/*
		if (ItemESP::Ammo)
		{
			for (auto actor : lootPawns)
			{
				UFortItemDefinition* definitions = (UFortItemDefinition*)ReadPointer(actor, FortOffsets::FortPickup::PrimaryPickupItemEntry + FortOffsets::FortItemEntry::ItemDefinition);
				if (!definitions) continue;
				wchar_t* displayname = definitions->DisplayName.Get();
				if (!displayname) continue;
				char LootTier = definitions->GetTier();

				std::wstring DisplayNameAsWString = displayname;
				std::string DisplayNameAsString(DisplayNameAsWString.begin(), DisplayNameAsWString.end());

				if (!IfFirstStringContains(DisplayNameAsString, XorString("Ammo: ")))
					continue;

				ImU32 Color;
				auto root = actor->GetRootComp();
				if (!root)
					continue;

				auto localroot = LocalPawn->GetRootComp();
				Vector3 RootPos = root->GetPos();
				Vector3 LootPos = RootPos;
				WorldToScreen(&LootPos.x);

				int DistanceToObject = (int)Math::GameDist(localroot->GetPos(), RootPos);

				if (IfFirstStringContains(DisplayNameAsString, XorString("Ammo: ")))
				{
					Color = ImGui::GetColorU32({ 255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1 });

					std::wstring DisplayName = std::wstring(displayname);
					std::string DistanceTo = std::to_string(DistanceToObject); DistanceTo += std::string(XorString(" m"));
					std::string FullAmmoESP;
					if (!IsPawnInVehicle())
						FullAmmoESP = std::string(DisplayName.begin(), DisplayName.end()) + XorString(" [") + DistanceTo + XorString("]");
					else
						FullAmmoESP = std::string(DisplayName.begin(), DisplayName.end());

					auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullAmmoESP.c_str());
					Text(ImVec2(LootPos.x - size.x / 2, LootPos.y - size.y / 2), Color, true, FullAmmoESP.c_str());
				}
				else
					continue;
			}
		}

		if (ItemESP::Materials)
		{
			for (auto actor : lootPawns)
			{
				UFortItemDefinition* definitions = (UFortItemDefinition*)ReadPointer(actor, FortOffsets::FortPickup::PrimaryPickupItemEntry + FortOffsets::FortItemEntry::ItemDefinition);

				if (!definitions) continue;

				wchar_t* displayname = definitions->DisplayName.Get();

				if (!displayname) continue;

				char LootTier = definitions->GetTier();

				std::wstring DisplayNameAsWString = displayname;
				std::string DisplayNameAsString(DisplayNameAsWString.begin(), DisplayNameAsWString.end());

				if (DisplayNameAsString != XorString("Wood"))
				{
					if (DisplayNameAsString != XorString("Stone"))
					{
						if (DisplayNameAsString != XorString("Metal"))
						{
							continue;
						}
					}
				}
				ImU32 Color;
				auto root = actor->GetRootComp();
				if (!root)
					continue;

				auto localroot = LocalPawn->GetRootComp();
				Vector3 RootPos = root->GetPos();
				Vector3 LootPos = RootPos;
				WorldToScreen(&LootPos.x);

				int DistanceToObject = (int)Math::GameDist(localroot->GetPos(), RootPos);

				if (DisplayNameAsString == XorString("Wood") || XorString("Stone") || XorString("Metal"))
				{
					Color = ImGui::GetColorU32({ 255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1 });
					std::wstring DisplayName = std::wstring(displayname);
					std::string DistanceTo = std::to_string(DistanceToObject); DistanceTo += std::string(XorString(" m"));
					std::string FullMaterialsESP;
					if (!IsPawnInVehicle())
						FullMaterialsESP = std::string(DisplayName.begin(), DisplayName.end()) + XorString(" [") + DistanceTo + XorString("]");
					else
						FullMaterialsESP = std::string(DisplayName.begin(), DisplayName.end());

					auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullMaterialsESP.c_str());
					Text(ImVec2(LootPos.x - size.x / 2, LootPos.y - size.y / 2), Color, true, FullMaterialsESP.c_str());
				}
				else
					continue;
			}
		}
		*/
	}

	float closestDistance = FLT_MAX;
	PVOID closestPawn = NULL;

	if (GlobalSettings::AllEnable && ItemESP::Enabled && ItemESP::WeakSpot)
	{
		for (auto actor : weakspotPawns)
		{
			player* pawn = (player*)actor;

			auto root = actor->GetRootComp();
			if (root)
			{
				auto localroot = LocalPawn->GetRootComp();
				Vector3 RootPos = root->GetPos();
				Vector3 ItemPos = RootPos;
				WorldToScreen(&ItemPos.x);

				auto dist = Math::FastDist2DVec3(Global::WndSize / 2, ItemPos);
				if (dist < Aimbot::Range && dist < closestDistance)
				{
					closestDistance = dist;
					closestPawn = pawn;
				}

				std::wstring DisplayName = XorString(L"WS");
				std::string FullWeakSpot;
				FullWeakSpot = XorString("[") + std::string(DisplayName.begin(), DisplayName.end()) + XorString("]");

				auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullWeakSpot.c_str());
				Text(ImVec2(ItemPos.x - size.x / 2, ItemPos.y - size.y / 2), ImGui::GetColorU32(Colours::Red), true, FullWeakSpot.c_str());
			}
		}
	}

	if (GlobalSettings::AllEnable && Aimbot::Enabled && Aimbot::DrawLine && !Global::Visible)
	{
		auto target = (player*)Global::Target;
		std::wstring ActorName = GetObjectFirstName((UObject*)target);

		if (wcsstr(ActorName.c_str(), XorString(L"PlayerPawn")))
		{
			auto mesh = target->GetMesh();
			auto root = target->GetRootComp();
			auto localroot = LocalPawn->GetRootComp();
			Vector3 RootPos = root->GetPos();
			if (mesh)
			{
				int DistanceToPlayer = (int)Math::GameDist(localroot->GetPos(), RootPos);
				if (DistanceToPlayer < Aimbot::DistanceLimit)
				{
					Vector3 AimBoneW2S = GetBoneWithRotation(mesh, Aimbot::Bone);
					WorldToScreen(&AimBoneW2S.x);
					if (PressedKeys[(int)Aimbot::Key])
					{
						window.DrawList->AddLine(ImVec2(Global::WndSize.x / 2, Global::WndSize.y / 2), ImVec2(AimBoneW2S.x, AimBoneW2S.y), ImGui::GetColorU32(Colours::Red), 0.5f);
					}
					else
					{
						window.DrawList->AddLine(ImVec2(Global::WndSize.x / 2, Global::WndSize.y / 2), ImVec2(AimBoneW2S.x, AimBoneW2S.y), ImGui::GetColorU32(Colours::White), 0.5f);
					}
				}
			}
		}
		if (wcsstr(ActorName.c_str(), XorString(L"WeakSpot")))
		{
			auto root = target->GetRootComp();
			Vector3 RootPos = root->GetPos();
			Vector3 ItemPos = RootPos;
			WorldToScreen(&ItemPos.x);

			if (root)
			{
				if (PressedKeys[(int)Aimbot::Key])
				{
					window.DrawList->AddLine(ImVec2(Global::WndSize.x / 2, Global::WndSize.y / 2), ImVec2(ItemPos.x, ItemPos.y), ImGui::GetColorU32(Colours::Red), 0.5f);
				}
				else
				{
					window.DrawList->AddLine(ImVec2(Global::WndSize.x / 2, Global::WndSize.y / 2), ImVec2(ItemPos.x, ItemPos.y), ImGui::GetColorU32(Colours::White), 0.5f);
				}
			}
		}
	}

	if (GlobalSettings::AllEnable && Aimbot::DrawLine && !Global::Visible)
	{
	}

	if ((GlobalSettings::AllEnable) && EnemyESP::Enabled || Aimbot::Enabled)
	{
		for (AActor* actor : playerPawns)
		{
			player* pawn = (player*)actor;
			USkeletalMeshComponent* mesh = pawn->GetMesh();

			if (mesh)
			{
				auto playerstate = pawn->GetPlayerState();
				if (playerstate)
				{
					bool InAVehicle = IsPawnInVehicle();

					if (GlobalSettings::AllEnable && Aimbot::Enabled)
					{
						if ((GlobalSettings::VisCheck && LineOfSightTo((PVOID)PlayerController, (PVOID)pawn, &Global::CameraLocation)) || !GlobalSettings::VisCheck)
						{
							Vector3 target = GetBoneWithRotation(mesh, (int)Aimbot::Bone);
							WorldToScreen(&target.x);

							auto dist = Math::FastDist2DVec3(Global::WndSize / 2, target);
							if (dist < Aimbot::Range && dist < closestDistance)
							{
								closestDistance = dist;
								closestPawn = pawn;
							}
						}
					}

					Vector3 head = GetBoneWithRotation(mesh, eBone::BONE_HEAD);
					WorldToScreen(&head.x);

					Vector3 chest = GetBoneWithRotation(mesh, eBone::BONE_CHEST);
					WorldToScreen(&chest.x);

					Vector3 lshoulder = GetBoneWithRotation(mesh, eBone::BONE_L_SHOULDER_2);
					WorldToScreen(&lshoulder.x);

					Vector3 rshoulder = GetBoneWithRotation(mesh, eBone::BONE_R_SHOULDER);
					WorldToScreen(&rshoulder.x);

					Vector3 lelbow = GetBoneWithRotation(mesh, eBone::BONE_L_ELBOW);
					WorldToScreen(&lelbow.x);

					Vector3 relbow = GetBoneWithRotation(mesh, eBone::BONE_R_ELBOW);
					WorldToScreen(&relbow.x);

					Vector3 lhand = GetBoneWithRotation(mesh, eBone::BONE_L_HAND_ROOT_1);
					WorldToScreen(&lhand.x);

					Vector3 rhand = GetBoneWithRotation(mesh, eBone::BONE_R_HAND_ROOT_1);
					WorldToScreen(&rhand.x);

					Vector3 pelvis = GetBoneWithRotation(mesh, eBone::BONE_PELVIS_1);
					WorldToScreen(&pelvis.x);

					Vector3 lleg = GetBoneWithRotation(mesh, eBone::BONE_L_LEG_ROOT);
					WorldToScreen(&lleg.x);

					Vector3 rleg = GetBoneWithRotation(mesh, eBone::BONE_R_LEG_ROOT);
					WorldToScreen(&rleg.x);

					Vector3 lknee = GetBoneWithRotation(mesh, eBone::BONE_L_KNEE);
					WorldToScreen(&lknee.x);

					Vector3 rknee = GetBoneWithRotation(mesh, eBone::BONE_R_KNEE);
					WorldToScreen(&rknee.x);

					Vector3 lfoot = GetBoneWithRotation(mesh, eBone::BONE_L_FOOT_MID);
					WorldToScreen(&lfoot.x);

					Vector3 rfoot = GetBoneWithRotation(mesh, eBone::BONE_R_FOOT_MID);
					WorldToScreen(&rfoot.x);

					std::list < Vector3 > bones = { head, chest, lshoulder, rshoulder, lelbow, relbow, lhand, rhand, pelvis, lleg, rleg, lknee, rknee, lfoot, rfoot };

					auto x0 = chest.x, x1 = chest.x, y0 = chest.y, y1 = chest.y;
					auto root = actor->GetRootComp();
					auto localroot = LocalPawn->GetRootComp();
					Vector3 RootPos = root->GetPos();
					int DistanceToPlayer = (int)Math::GameDist(localroot->GetPos(), RootPos);

					if (EnemyESP::Enabled && EnemyESP::Box || EnemyESP::Skeleton)
					{
						for (auto a : bones)
						{
							x0 = min(x0, a.x);
							x1 = max(x1, a.x);

							y0 = min(y0, a.y);
							y1 = max(y1, a.y);
						}

						x0 = std::int32_t(std::round(x0));
						y0 = std::int32_t(std::round(y0));

						x1 = std::int32_t(std::round(x1));
						y1 = std::int32_t(std::round(y1));

						x0 -= (x1 - x0) * 0.45f;
						y0 -= (y1 - y0) * 0.15f;

						x1 += (x1 - x0) * 0.45f;
						y1 += (y1 - y0) * 0.15f;
					}

					if (GlobalSettings::AllEnable && EnemyESP::Enabled && EnemyESP::Box)
					{
						if (!InAVehicle && DistanceToPlayer > EnemyESP::DistanceLimit)
							continue;

						ImVec4 BoundingBoxESPColourAfterChecks;
						if (GlobalSettings::VisCheck)
						{
							if (LineOfSightTo((PVOID)PlayerController, (PVOID)pawn, &Global::CameraLocation))
							{
								BoundingBoxESPColourAfterChecks = Colours::BoundingBoxESPColourVisible;
							}
							else
								BoundingBoxESPColourAfterChecks = Colours::BoundingBoxESPColour;
						}
						else
							BoundingBoxESPColourAfterChecks = Colours::BoundingBoxESPColour;

						window.DrawList->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), ImGui::GetColorU32(BoundingBoxESPColourAfterChecks));
					}

					if (GlobalSettings::AllEnable && EnemyESP::Enabled && EnemyESP::Skeleton)
					{
						if (!InAVehicle && DistanceToPlayer > EnemyESP::DistanceLimit)
							continue;

						ImVec4 SkeletonESPColourAfterChecks = Colours::SkeletonESPColour;

						window.DrawList->AddLine(ImVec2(head.x, head.y), ImVec2(chest.x, chest.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(chest.x, chest.y), ImVec2(lshoulder.x, lshoulder.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(lshoulder.x, lshoulder.y), ImVec2(lelbow.x, lelbow.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(lelbow.x, lelbow.y), ImVec2(lhand.x, lhand.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(chest.x, chest.y), ImVec2(rshoulder.x, rshoulder.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(rshoulder.x, rshoulder.y), ImVec2(relbow.x, relbow.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(relbow.x, relbow.y), ImVec2(rhand.x, rhand.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(chest.x, chest.y), ImVec2(pelvis.x, pelvis.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(pelvis.x, pelvis.y), ImVec2(lleg.x, lleg.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(lleg.x, lleg.y), ImVec2(lknee.x, lknee.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(lknee.x, lknee.y), ImVec2(lfoot.x, lfoot.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(pelvis.x, pelvis.y), ImVec2(rleg.x, rleg.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(rleg.x, rleg.y), ImVec2(rknee.x, rknee.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
						window.DrawList->AddLine(ImVec2(rknee.x, rknee.y), ImVec2(rfoot.x, rfoot.y), ImGui::GetColorU32(SkeletonESPColourAfterChecks), EnemyESP::SkeletonThickness);
					}

					if (GlobalSettings::AllEnable && EnemyESP::Enabled && EnemyESP::Name)
					{
						if (!InAVehicle && DistanceToPlayer > EnemyESP::DistanceLimit)
							continue;

						if (pawn->GetActorID() != LocalPawn->GetActorID())
						{
							if (InAVehicle)
							{
								auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, XorString("Bot"));
								//Text(ImVec2((x0 + x1) / 2 - size.x / 2, y0 - 16), ImGui::GetColorU32(Colours::GlobalTextColour), true, XorString("Bot")); // Top of player
								Text(ImVec2((x0 + x1) / 2 - size.x / 2, y1 + 4), ImGui::GetColorU32(Colours::GlobalTextColour), true, XorString("Bot")); // Bottom of player
							}
							else
							{
								if (EnemyESP::Distance)
								{
									std::string Name = XorString("Bot");
									std::string distance;
									distance += std::to_string((int)Math::GameDist(localroot->GetPos(), root->GetPos())); distance += std::string(XorString(" m"));
									std::string FullESPLine = Name;
									FullESPLine += XorString(" [") + distance + XorString("]");

									auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullESPLine.c_str());
									//Text(ImVec2((x0 + x1) / 2 - size.x / 2, y0 - 16), ImGui::GetColorU32(Colours::GlobalTextColour), true, FullESPLine.c_str()); // Top of player
									Text(ImVec2((x0 + x1) / 2 - size.x / 2, y1 + 4), ImGui::GetColorU32(Colours::GlobalTextColour), true, FullESPLine.c_str()); // Bottom of player
								}
								else
								{
									auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, XorString("Bot"));
									//Text(ImVec2((x0 + x1) / 2 - size.x / 2, y0 - 16), ImGui::GetColorU32(Colours::GlobalTextColour), true, XorString("Bot")); // Top of player
									Text(ImVec2((x0 + x1) / 2 - size.x / 2, y1 + 4), ImGui::GetColorU32(Colours::GlobalTextColour), true, XorString("Bot")); // Bottom of player
								}
							}
						}
						else
						{
							FString playerName;
							SpoofCall(ProcessEvent, playerstate, GetPlayerName, &playerName, 0);
							if (playerName.c_str())
							{
								CHAR copy[0xFF] = { 0 };
								wcstombs(copy, playerName.c_str(), sizeof(copy));
								Free(playerName.c_str());

								if (!InAVehicle)
								{
									if (EnemyESP::Distance)
									{
										std::string distance;
										distance += std::to_string((int)Math::GameDist(localroot->GetPos(), root->GetPos())); distance += std::string(XorString(" m"));

										std::string FullESPLine = copy;
										FullESPLine += XorString(" [") + distance + XorString("]");

										auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, FullESPLine.c_str());
										//Text(ImVec2((x0 + x1) / 2 - size.x / 2, y0 - 16), ImGui::GetColorU32(Colours::GlobalTextColour), true, FullESPLine.c_str()); // Top of player
										Text(ImVec2((x0 + x1) / 2 - size.x / 2, y1 + 4), ImGui::GetColorU32(Colours::GlobalTextColour), true, FullESPLine.c_str()); // Bottom of player
									}
								}
								else
								{
									auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, copy);
									//Text(ImVec2((x0 + x1) / 2 - size.x / 2, y0 - 16), ImGui::GetColorU32(Colours::GlobalTextColour), true, copy); // Top of player
									Text(ImVec2((x0 + x1) / 2 - size.x / 2, y1 + 4), ImGui::GetColorU32(Colours::GlobalTextColour), true, copy); // Bottom of player
								}
							}
						}
					}
				}
			}
		}
	}

	if (GlobalSettings::AllEnable)
	{
		if (Aimbot::DrawFov)
			window.DrawList->AddCircle(ImVec2(Global::WndSize.x / 2, Global::WndSize.y / 2), Aimbot::Range, ImGui::GetColorU32(Colours::FOVCircleColour), 35);

		Global::Target = closestPawn;
	}
	looped = false;

	std::string TextWatermark = TextFormat(XorString("%s"), reseller_name);
	float PositionXWatermark = 60;
	float PositionYWatermark = 65;
	ImU32 WaterMarkColor = ImGui::GetColorU32({ 255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1 });
	Text(ImVec2(PositionXWatermark, PositionYWatermark), WaterMarkColor, true, TextWatermark.c_str());

	if (GlobalSettings::AllEnable && ItemESP::Weapons)
	{
		ImU32 LootTextScreenColor = ImGui::GetColorU32({ 101.f / 255.f, 103.f / 255.f, 105.f / 255.f, 1 });
		std::string TextLoot = XorString("Minimum loot tier (Page Up/Down): ");
		switch (ItemESP::Tier)
		{
		case 0: // Common - Grey
			LootTextScreenColor = ImGui::GetColorU32({ 168.f / 255.f, 168.f / 255.f, 168.f / 255.f, 1 });
			TextLoot += XorString("Common");
			break;
		case 1: // Uncommon - Green
			LootTextScreenColor = ImGui::GetColorU32({ 7.f / 255.f, 143.f / 255.f, 4.f / 255.f, 1 });
			TextLoot += XorString("Uncommon");
			break;
		case 2: // Rare - Blue
			LootTextScreenColor = ImGui::GetColorU32({ 8.f / 255.f, 1.f / 255.f, 255.f / 255.f, 1 });
			TextLoot += XorString("Rare");
			break;
		case 3: // Epic - Purple
			LootTextScreenColor = ImGui::GetColorU32({ 112.f / 255.f, 7.f / 255.f, 135.f / 255.f, 1 });
			TextLoot += XorString("Epic");
			break;
		case 4: // Legendary - Gold
			LootTextScreenColor = ImGui::GetColorU32({ 255.f / 255.f, 200.f / 255.f, 0.f / 255.f, 1 });
			TextLoot += XorString("Legendary");
			break;
		case 5: // Mythic - Dark Gold
			LootTextScreenColor = ImGui::GetColorU32({ 255.f / 255.f, 150.f / 255.f, 0.f / 255.f, 1 });
			TextLoot += XorString("Mythic");
			break;
		}
		float PositionXLootText = 40;
		float PositionYLootText = 350;
		Text(ImVec2(PositionXLootText, PositionYLootText), ImGui::GetColorU32(LootTextScreenColor), true, TextLoot.c_str());
	}

	EndScene(window);

	return PresentOriginal(swapChain, syncInterval, flags);
}

__declspec(dllexport) HRESULT ResizeHook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags)
{
	ImGui_ImplDX11_Shutdown();
	renderTargetView->Release();
	immediateContext->Release();
	device->Release();
	device = nullptr;

	return ResizeOriginal(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
}
ImGuiWindow& BeginScene()
{
	ImGui_ImplDX11_NewFrame();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin("##scene", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);

	auto& io = ImGui::GetIO();
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

	return *ImGui::GetCurrentWindow();
}
size_t CurrentTabOpen = 0;
void AddTab(size_t Index, const char* Text)
{
	static const size_t TabWidth = 147;
	static const size_t TabHeight = 18;

	ImGui::PushID(Index);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);

	if (Index == 1)
		ImGui::SameLine(Index * (TabWidth + 5));
	else if (Index > 1)
		ImGui::SameLine(Index * (TabWidth + 4 - Index));

	if (CurrentTabOpen == Index)
		//ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(15, 15, 15));			// Color on tab open
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(67, 78, 161));			// Color on tab open
	else
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(79, 95, 214));			// Color on tab closed
		//ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(30, 30, 30));			// Color on tab closed

	//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(40, 40, 40));		// Color on mouse hover in tab
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(67, 78, 161));		// Color on mouse hover in tab

	//ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(35, 35, 35));		// Color on click tab
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(37, 44, 99));			// Color on click tab

	if (ImGui::Button(Text, ImVec2(TabWidth, TabHeight)))								// If tab clicked
		CurrentTabOpen = Index;

	ImGui::PopStyleVar();
	ImGui::PopStyleColor(3);
	ImGui::PopID();
}
VOID EndScene(ImGuiWindow& window)
{
	//ColorChange();

	window.DrawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::Render();
}

VOID Main()
{
	IDXGISwapChain* swapChain = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	auto featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = FindWindowA(XorString("UnrealWindow"), XorString("Fortnite  "));
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, 0, &featureLevel, 1, D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, &context)))
	{
		MessageBox(0, XorString(L"Failed to create D3D11 device and swap chain"), XorString(L"Failure"), MB_ICONERROR);
		return;
	}

	auto table = *reinterpret_cast<PVOID**>(swapChain);
	auto present = table[8];
	auto resize = table[13];

	context->Release();
	device->Release();
	swapChain->Release();

	//auto dxgi = GetModuleHandleA("dxgi.dll";
	//auto present = FindPattern("48 89 ?? 24 ?? 48 89 ?? 24 ?? ?? ?? ?? ?? 48 8d 6c 24 ?? 48 81 ec ?? ?? 00 00 48 8b 05 ?? ?? ?? ?? 48 33 C4 48 89 45 ?? 45 33 F6 44 89 44 24 ?? 44 39 35 ?? ?? ?? ?? 41 8b F0 8b fa", (const char*)dxgi;
	//auto resize = FindPattern("48 8B C4 55 41 54 41 55 41 56 41 57 48 8d 68 ?? 48 81 ec ?? ?? 00 00 48 c7 45 ?? FE FF FF FF 48 89 ?? ?? 48 89 ?? ?? 48 89 ?? ?? 45 8b f9 45 8b e0 44 8b ea", (const char*)dxgi;

	InitializeOffsets();

	const auto pcall_present_discord_present = Helper::PatternScan(Discord::GetDiscordModuleBase(), XorString("FF 15 ? ? ? ? 8B D8 E8 ? ? ? ? E8 ? ? ? ? EB 10"));
	if (!pcall_present_discord_present)
	{
		MessageBox(0, XorString(L"Error 1B"), 0, 0);
	}
	const auto poriginal_present = reinterpret_cast<f_present*>(pcall_present_discord_present + *reinterpret_cast<int32_t*>(pcall_present_discord_present + 0x2) + 0x6);
	if (!*poriginal_present)
	{
		MessageBox(0, XorString(L"Error 1B"), 0, 0);
	}
	PresentOriginal = *poriginal_present;
	*poriginal_present = PresentHook;

	//DISCORD.HookFunction((uintptr_t)resize, (uintptr_t)ResizeHook, (uintptr_t)&ResizeOriginal);
	DISCORD.HookFunction((uintptr_t)present, (uintptr_t)PresentHook, (uintptr_t)&poriginal_present);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		//AllocConsole();
		//FILE* fDummy;
		//freopen_s(&fDummy, "CONOUT$", "w", stdout);
		//freopen_s(&fDummy, "CONOUT$", "w", stderr);
		//freopen_s(&fDummy, "CONIN$", "r", stdin);

		FORTNITE_UTILS::SetSpoofStub((uint8_t*)Helper::PatternScan((uintptr_t)GetModuleHandleA(nullptr), XorString("FF 27")));
		Main();
	}
	return TRUE;
}