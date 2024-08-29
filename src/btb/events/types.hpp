#pragma once
#include "../../pch.hpp"

extern nes::event_dispatcher btb_dispatcher;

struct Color {
	float r, g, b, a;
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {};
};

namespace btb {
	namespace event_types {
		struct key_event {
			bool mCancel = false;
			int key;
			bool down;
		};

		struct fov_event {
			bool mCancel = false;
			float fov;
		};

		struct mouse_event {
			bool mCancel = false;
			void* mouseDevice;
			char button;
			char action;
			uintptr_t mouseX;
			uintptr_t mouseY;
			uintptr_t movementX;
			uintptr_t movementY;
			char a8;
		};

		struct gamma_event {
			bool mCancel = false;
			float gamma;
		};

		struct present_Event {
			bool mCancel = false;
			IDXGISwapChain* swapChain;
			UINT syncInterval;
			UINT flags;
		};
	};
};