#pragma once
#include "../../../pch.hpp"

class DirectXHooks {
private:
	static HRESULT IDXGISwapChain_Present(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags);
	static HRESULT IDXGISwapChain_ResizeBuffers(IDXGISwapChain3* swap_chain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
public:
	static void Init();
};