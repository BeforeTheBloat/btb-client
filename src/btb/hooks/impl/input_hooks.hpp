#pragma once
#include "../../../pch.hpp"

class InputHooks {
private:
	static void Keyboard_Feed(int key, bool down);
	static void MouseDevice_Feed(void* mouseDevice, char button, char action, uintptr_t mouseX, uintptr_t mouseY, uintptr_t movementX, uintptr_t movementY, char a8);
public:
	static void Init();
};