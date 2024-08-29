#include "minecraft_visual_hooks.hpp"

std::unique_ptr<btb::hook_type> get_fov;
std::unique_ptr<btb::hook_type> get_gamma;

float MinecraftVisualHooks::LevelRendererPlayer_GetFOV(void* a1, void* a2, void* a3, void* a4) {
	if (get_fov) {
		auto ofunc = get_fov->getOriginal<float(*)(void*, void*, void*, void*)>();
		float fov = ofunc(a1, a2, a3, a4);
		
		nes::event_holder<btb::event_types::fov_event> event;
		event->fov = fov;
		btb_dispatcher.trigger(event);

		return fov;
	}
}

float MinecraftVisualHooks::Options_getGamma(void* a1, void* a2) {
	if (get_gamma) {
		auto ofunc = get_gamma->getOriginal<float(*)(void*, void*)>();
		float gamma = ofunc(a1, a2);

		nes::event_holder<btb::event_types::gamma_event> event;
		event->gamma = gamma;
		btb_dispatcher.trigger(event);

		return gamma;
	}
}

void MinecraftVisualHooks::Init() {
	uintptr_t getfovaddr = btb::get_signature("? ? ? ? ? ? ? 48 89 ? ? 57 48 81 EC ? ? ? ? 0F 29 ? ? 0F 29 ? ? 44 0F ? ? ? 44 0F ? ? ? 48 8B ? ? ? ? ? 48 33 ? 48 89 ? ? ? 41 0F");
	get_fov = std::make_unique<btb::hook_type>(getfovaddr, reinterpret_cast<void*>(&MinecraftVisualHooks::LevelRendererPlayer_GetFOV));
	get_fov->enable();

	uintptr_t getgammaaddr = btb::get_signature("48 83 EC ? 80 B9 ? ? ? ? ? 48 8D 54 24 ? 48 8B 01 48 8B 40 ? 74 ? 41 B8 ? ? ? ? FF 15 ? ? ? ? 48 8B 10 48 85 D2 74 ? 48 8B 42 ? 48 8B 88 ? ? ? ? 48 85 C9 74 ? E8 ? ? ? ? 48 83 C4 ? C3 F3 0F 10 42 ? 48 83 C4 ? C3 41 B8 ? ? ? ? FF 15 ? ? ? ? 48 8B 10 48 85 D2 75 ? E8 ? ? ? ? CC E8 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC 40 53 48 83 EC ? 48 8B 01");
	get_gamma = std::make_unique<btb::hook_type>(getgammaaddr, reinterpret_cast<void*>(&MinecraftVisualHooks::Options_getGamma));
	get_gamma->enable();
}