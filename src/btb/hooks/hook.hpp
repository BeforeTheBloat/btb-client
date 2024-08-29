#pragma once
#include "../../pch.hpp"

namespace btb {
	using hook_func_ptr = void*;

	class hook_type {
	private:
		void* functionPointer;
		void* functionDetour;
		uintptr_t functionTarget;
		bool isVtable;
	public:
		hook_type(uintptr_t target, void* detour, bool isVtable = false);
		~hook_type() = default;

		void enable() {
			MH_EnableHook(reinterpret_cast<void*>(functionTarget));
		}

		void disable() {
			MH_DisableHook(reinterpret_cast<void*>(functionTarget));
		}

		template <typename FunctionType>
		FunctionType getOriginal() {
			return reinterpret_cast<FunctionType>(this->functionPointer);
		}
	};
};