#pragma once
#include "pch.hpp"

using namespace winrt::Windows::UI::Notifications;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::System;

void createConsole() {
    HWND consoleWnd = GetConsoleWindow();
    if (!consoleWnd) {
        AllocConsole();

        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONIN$", "r", stdin);
        freopen_s(&f, "CONERR$", "w", stderr);

        SetConsoleTitleA("btb-client");
        ShowWindow(GetConsoleWindow(), SW_SHOW);
    }
    else {
        ShowWindow(consoleWnd, SW_SHOW);
    }
}

BOOL APIENTRY BTBMain(HMODULE hModule) {
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    const winrt::Windows::ApplicationModel::Package package = winrt::Windows::ApplicationModel::Package::Current();
    auto [major, minor, build, revision] = package.Id().Version();

    std::wstring version = std::to_wstring(major) + L"." + std::to_wstring(minor) + L"." + std::to_wstring(build) + L"." + std::to_wstring(revision);

    {
        std::wstring build_str = std::to_wstring(build);
        if (build_str.length() > 2) {
            build_str = build_str.substr(0, build_str.length() - 2);
        }

        std::wstring title_version = std::to_wstring(major) + L"." + std::to_wstring(minor) + L"." + build_str;
        std::wstring title = L"Before The Bloat v1.0.0 (v" + title_version + L")";
        CoreApplication::MainView().CoreWindow().DispatcherQueue().TryEnqueue([title = std::move(title)]() {
            ApplicationView::GetForCurrentView().Title(title);
            });
    }

    // Debug Console
    createConsole();
    std::cout << minor << " " << build << " " << revision << std::endl;

    MH_Initialize();

    DirectXHooks::Init();
    InputHooks::Init();
    MinecraftVisualHooks::Init();
    
    return TRUE;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)BTBMain, hModule, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
