#include "input_hooks.hpp"

std::unique_ptr<btb::hook_type> input_hook;
std::unique_ptr<btb::hook_type> mouse_hook;

void InputHooks::Keyboard_Feed(int key, bool down) {
	if (input_hook) {
		auto ofunc = input_hook->getOriginal<void(*)(int, bool)>();

		nes::event_holder<btb::event_types::key_event> event;
		event->key = key;
		event->down = down;
		btb_dispatcher.trigger(event);

		ofunc(key, down);
	}
}

void InputHooks::MouseDevice_Feed(void* mouseDevice, char button, char action, uintptr_t mouseX, uintptr_t mouseY, uintptr_t movementX, uintptr_t movementY, char a8) {
	if (mouse_hook) {
		auto ofunc = mouse_hook->getOriginal<void(*)(void*, char, char, uintptr_t, uintptr_t, uintptr_t, uintptr_t, char)>();

		nes::event_holder<btb::event_types::mouse_event> event;
		event->mouseDevice = mouseDevice;
		event->button = button;
		event->action = action;
		event->mouseX = mouseX;
		event->mouseY = mouseY;
		event->movementX = movementX;
		event->movementY = movementY;
		event->a8 = a8;

		btb_dispatcher.trigger(event);

		ofunc(mouseDevice, button, action, mouseX, mouseY, movementX, movementY, a8);
	}
}

void InputHooks::Init() {
	uintptr_t keyaddr = btb::get_signature("Keyboard::feed");
	input_hook = std::make_unique<btb::hook_type>(keyaddr, reinterpret_cast<void*>(&InputHooks::Keyboard_Feed));
	input_hook->enable();

	uintptr_t mouseaddr = btb::get_signature("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 44 0F B7 BC 24");
	mouse_hook = std::make_unique<btb::hook_type>(mouseaddr, reinterpret_cast<void*>(&InputHooks::MouseDevice_Feed));
	mouse_hook->enable();
}