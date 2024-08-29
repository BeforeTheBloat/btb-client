#pragma once
#include "../../../pch.hpp"

class MinecraftVisualHooks {
private:
	static float LevelRendererPlayer_GetFOV(void* a1, void* a2, void* a3, void* a4);
	static float Options_getGamma(void* a1, void* a2);
public:
	static void Init();
};