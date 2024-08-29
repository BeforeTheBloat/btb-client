#include "directx_hooks.hpp"

typedef HRESULT(__thiscall* PresentType)(IDXGISwapChain3*, UINT, UINT);
PresentType PresentOriginal;

typedef HRESULT(__thiscall* ResizeType)(IDXGISwapChain3*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
ResizeType ResizeOriginal;

ID3D11Device* d3d11Device = nullptr;
bool init = false;

static std::chrono::high_resolution_clock fpsclock;
static std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
int frames = 0;
float fps = 0;

HRESULT DirectXHooks::IDXGISwapChain_Present(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags) {
    std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now() - start;
    frames += 1;

    if (elapsed.count() >= 0.5f) {
        fps = static_cast<int>((float)frames / elapsed.count());

        frames = 0;
        start = std::chrono::high_resolution_clock::now();
    }

    swapChain->GetDevice(IID_PPV_ARGS(&d3d11Device));

    ID3D11DeviceContext* ppContext = nullptr;
    d3d11Device->GetImmediateContext(&ppContext);

    ID3D11Texture2D* pBackBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    ID3D11RenderTargetView* mainRenderTargetView;
    d3d11Device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);

    pBackBuffer->Release();

    if (!init) {
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(FindWindowA(nullptr, "Minecraft: Windows 10 Edition"));
        ImGui_ImplDX11_Init(d3d11Device, ppContext);
        init = true;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    ImGui::Begin("FPS");
    ImGui::Text("FPS: %.2f", fps);
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();

    ppContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    mainRenderTargetView->Release();
    d3d11Device->Release();
    ppContext->Release();

    return PresentOriginal(swapChain, syncInterval, flags);
}

HRESULT DirectXHooks::IDXGISwapChain_ResizeBuffers(IDXGISwapChain3* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
    return ResizeOriginal(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
}

void DirectXHooks::Init() {
    if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
        kiero::bind(8, (void**)&PresentOriginal, DirectXHooks::IDXGISwapChain_Present);
        kiero::bind(13, (void**)&ResizeOriginal, DirectXHooks::IDXGISwapChain_ResizeBuffers);
        return;
    }
}