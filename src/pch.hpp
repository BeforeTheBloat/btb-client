#pragma once

// C/C++ Includes
#include <algorithm>
#include <any>
#include <atomic>
#include <bitset>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// Windows API
#include <Windows.h>
#include <Psapi.h>
#include <d3d11.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

// WinRT
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Applicationmodel.core.h>
#include <winrt/Windows.UI.ViewManagement.h>
#undef GetCurrentTime
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>

// Libraries
#include <libhat.hpp>
#include <MinHook.h>
#include <magic_enum.hpp>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nes/event_dispatcher.hpp>

#include "utils/imgui/imgui.h"
#include "utils/imgui/imgui_impl_dx11.h"
#include "utils/imgui/imgui_impl_win32.h"
#include "utils/kiero.h"

// Core
#include "btb/hooks/patch_helper.hpp"
#include "btb/hooks/hook.hpp"
#include "btb/events/types.hpp"

// Core Hooks
#include "btb/hooks/impl/input_hooks.hpp"
#include "btb/hooks/impl/minecraft_visual_hooks.hpp"
#include "btb/hooks/impl/directx_hooks.hpp"

// Versions
#include "btb/sdk/multiversion.hpp"