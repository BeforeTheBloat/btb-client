#include "hook.hpp"

btb::hook_type::hook_type(uintptr_t target, void* detour, bool isVtable) {
	functionTarget = target;
	functionDetour = detour;
	isVtable = isVtable;

	if (isVtable) {
		//impl coming soon
	}
	else {
		MH_CreateHook(reinterpret_cast<LPVOID>(functionTarget), functionDetour, &this->functionPointer);
	}
}