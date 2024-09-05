#include "directx_hooks.hpp"

typedef HRESULT(__thiscall* PresentType)(IDXGISwapChain3*, UINT, UINT);
PresentType PresentOriginal;

typedef HRESULT(__thiscall* ResizeType)(IDXGISwapChain3*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
ResizeType ResizeOriginal;

ID3D11Device* d3d11Device = nullptr;
ID3D11DeviceContext* ppContext = nullptr;
ID3D11RenderTargetView* mainRenderTargetView = nullptr;
ID3D11Texture2D* pBackBuffer = nullptr;

ID3D11ShaderResourceView* pOrigShaderResourceView = nullptr;
ID3D11RasterizerState* pRasterizerState = nullptr;
ID3D11DepthStencilState* pDepthStencilState = nullptr;

bool init = false;
static std::chrono::high_resolution_clock fpsclock;
static std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
int frames = 0;
float fps = 0;

using namespace DirectX;

struct BlurInputBuffer
{
    XMFLOAT2 resolution;
    XMFLOAT2 offset;
    XMFLOAT2 halfpixel;
    XMFLOAT2 _dummy;
};

ID3D11PixelShader* dbgShader;
ID3D11PixelShader* pUpsampleShader;
ID3D11PixelShader* pDownsampleShader;
ID3D11VertexShader* pVertexShader;
ID3D11InputLayout* pInputLayout;

ID3D11SamplerState* pSampler;
ID3D11Buffer* pVertexBuffer;
ID3D11Buffer* pConstantBuffer;
BlurInputBuffer constantBuffer;

#define BLUR_OFFSET 1.0f

static const XMFLOAT4 quadVertices[] =
{
    XMFLOAT4(1.0, -1.0, 0.0, 1.0),
    XMFLOAT4(-1.0, -1.0, 0.0, 1.0),
    XMFLOAT4(-1.0, 1.0, 0.0, 1.0),
    XMFLOAT4(-1.0, 1.0, 0.0, 1.0),
    XMFLOAT4(1.0, 1.0, 0.0, 1.0),
    XMFLOAT4(1.0, -1.0, 0.0, 1.0) };

const char* vertexShaderSrc = "struct VS_INPUT {\
    float4 pos : POSITION;\
};\
\
float4 main(VS_INPUT input) : SV_POSITION {\
    return input.pos;\
}";
const char* downsampleShaderSrc = "cbuffer BlurInputBuffer : register(b0)\
{\
    float2 resolution;\
    float2 offset;\
    float2 halfpixel;\
};\
sampler sampler0 : register(s0);\
Texture2D texture0 : register(t0);\
\
float4 main(float4 screenSpace : SV_Position) : SV_TARGET {\
    float2 uv = screenSpace.xy / resolution;\
    float4 sum = texture0.Sample(sampler0, uv) * 4.0;\
    sum += texture0.Sample(sampler0, uv - halfpixel * offset);\
    sum += texture0.Sample(sampler0, uv + halfpixel * offset);\
    sum += texture0.Sample(sampler0, uv + float2(halfpixel.x, -halfpixel.y) * offset);\
    sum += texture0.Sample(sampler0, uv - float2(halfpixel.x, -halfpixel.y) * offset);\
    return sum / 8.0;\
}";
const char* upsampleShaderSrc = "cbuffer BlurInputBuffer : register(b0)\
{\
    float2 resolution;\
    float2 offset;\
    float2 halfpixel;\
};\
struct PS_INPUT {\
    float4 pos : POSITION;\
};\
sampler sampler0 : register(s0);\
Texture2D texture0 : register(t0);\
\
float4 main(PS_INPUT input, float4 screenSpace : SV_Position) : SV_TARGET {\
    float2 uv = screenSpace.xy / resolution;\
    float4 sum = texture0.Sample(sampler0, uv + float2(-halfpixel.x * 2.0, 0.0) * offset);\
    sum += texture0.Sample(sampler0, uv + float2(-halfpixel.x, halfpixel.y) * offset) * 2.0;\
    sum += texture0.Sample(sampler0, uv + float2(0.0, halfpixel.y * 2.0) * offset);\
    sum += texture0.Sample(sampler0, uv + float2(halfpixel.x, halfpixel.y) * offset) * 2.0;\
    sum += texture0.Sample(sampler0, uv + float2(halfpixel.x * 2.0, 0.0) * offset);\
    sum += texture0.Sample(sampler0, uv + float2(halfpixel.x, -halfpixel.y) * offset) * 2.0;\
    sum += texture0.Sample(sampler0, uv + float2(0.0, -halfpixel.y * 2.0) * offset);\
    sum += texture0.Sample(sampler0, uv + float2(-halfpixel.x, -halfpixel.y) * offset) * 2.0;\
    return sum / 12.0;\
}";
const char* dbgDrawTextureShaderSrc = "cbuffer BlurInputBuffer : register(b0)\
{\
    float2 resolution;\
    float2 offset;\
    float2 halfpixel;\
};\
struct PS_INPUT {\
    float4 pos : POSITION;\
};\
sampler sampler0 : register(s0);\
Texture2D texture0 : register(t0);\
\
float4 main(PS_INPUT input, float4 screenSpace : SV_Position) : SV_TARGET {\
    float2 uv = screenSpace.xy / resolution;\
    return texture0.Sample(sampler0, uv);\
}";

ID3DBlob* TryCompileShader(const char* pSrcData, const char* pTarget)
{
    HRESULT hr;

    ID3DBlob* shaderBlob;
    ID3DBlob* errorBlob;
    hr = D3DCompile(pSrcData, strlen(pSrcData), nullptr, nullptr, nullptr, "main", pTarget, 0, 0, &shaderBlob, &errorBlob);

    return shaderBlob;
}

void InitializePipeline() {
    HRESULT hr;

    // byteWidth has to be a multiple of 32, BlurInputBuffer has a size of 24
    CD3D11_BUFFER_DESC cbd(
        sizeof(BlurInputBuffer),
        D3D11_BIND_CONSTANT_BUFFER);
    CD3D11_BUFFER_DESC cbdVertex(
        sizeof(quadVertices),
        D3D11_BIND_VERTEX_BUFFER);

    d3d11Device->CreateBuffer(
        &cbd,
        nullptr,
        &pConstantBuffer);

    D3D11_SUBRESOURCE_DATA vertexBufferData = { quadVertices, 0, 0 };

    d3d11Device->CreateBuffer(
        &cbdVertex,
        &vertexBufferData,
        &pVertexBuffer);

    ID3DBlob* shaderBlob = TryCompileShader(upsampleShaderSrc, "ps_4_0");
    d3d11Device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &pUpsampleShader);

    shaderBlob = TryCompileShader(downsampleShaderSrc, "ps_4_0");
    d3d11Device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &pDownsampleShader);

    shaderBlob = TryCompileShader(dbgDrawTextureShaderSrc, "ps_4_0");
    d3d11Device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &dbgShader);

    shaderBlob = TryCompileShader(vertexShaderSrc, "vs_4_0");
    d3d11Device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &pVertexShader);

    D3D11_INPUT_ELEMENT_DESC ied =
    { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
     0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    d3d11Device->CreateInputLayout(&ied, 1, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &pInputLayout);
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    d3d11Device->CreateSamplerState(&sd, &pSampler);

    d3d11Device->GetImmediateContext(&ppContext);
}

void RenderToRTV(ID3D11RenderTargetView* mainRenderTargetView, ID3D11ShaderResourceView* pShaderResourceView, XMFLOAT2 rtvSize) {
    HRESULT hr;

    D3D11_DEPTH_STENCIL_DESC dsd{};
    dsd.DepthEnable = false;
    dsd.StencilEnable = false;
    d3d11Device->CreateDepthStencilState(&dsd, &pDepthStencilState);
    ppContext->OMSetDepthStencilState(pDepthStencilState, 0);

    void* null = nullptr;
    ppContext->PSSetShaderResources(0, 1, reinterpret_cast<ID3D11ShaderResourceView**>(&null));
    ppContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);

    constantBuffer.resolution = rtvSize;
    constantBuffer.halfpixel = XMFLOAT2(0.5 / rtvSize.x, 0.5 / rtvSize.y);
    ppContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &constantBuffer, 0, 0);

    ppContext->IASetInputLayout(pInputLayout);
    UINT stride = sizeof(XMFLOAT4);
    UINT offset = 0;

    ppContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    ppContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ppContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    ppContext->VSSetShader(pVertexShader, nullptr, 0);
    ppContext->PSSetSamplers(0, 1, &pSampler);
    ppContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);
    D3D11_BLEND_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.AlphaToCoverageEnable = false;
    bd.RenderTarget[0].BlendEnable = true;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ID3D11BlendState* pBlendState;
    d3d11Device->CreateBlendState(&bd, &pBlendState);
    ppContext->OMSetBlendState(pBlendState, NULL, 0xffffffff);
    D3D11_RASTERIZER_DESC rd{};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = false;
    rd.ScissorEnable = false;
    d3d11Device->CreateRasterizerState(&rd, &pRasterizerState);
    ppContext->RSSetState(pRasterizerState);

    ppContext->PSSetShaderResources(0, 1, &pShaderResourceView);
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = rtvSize.x;
    viewport.Height = rtvSize.y;
    viewport.MaxDepth = 1.0f;

    // FLOAT backgroundColor[4] = {0.1f, 0.2f, 0.6f, 1.0f};
    // ppContext->ClearRenderTargetView(mainRenderTargetView, backgroundColor);
    ppContext->RSSetViewports(1, &viewport);
    ppContext->Draw(sizeof(quadVertices) / sizeof(quadVertices[0]), 0);
    ppContext->OMSetRenderTargets(1, reinterpret_cast<ID3D11RenderTargetView**>(&null), nullptr);
    

}

void RenderBlur(ID3D11Texture2D* pTextureToBlur, ID3D11RenderTargetView* pDstRenderTargetView, int iterations) {
    HRESULT hr;
    std::vector<ID3D11Texture2D*> framebuffers;
    std::vector<ID3D11RenderTargetView*> renderTargetViews;
    std::vector<ID3D11ShaderResourceView*> shaderResourceViews;
    std::vector<XMFLOAT2> fbSizes;
    D3D11_TEXTURE2D_DESC desc;
    pTextureToBlur->GetDesc(&desc);

    framebuffers.reserve((size_t)iterations);
    renderTargetViews.reserve((size_t)iterations);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.Format = desc.Format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    d3d11Device->CreateShaderResourceView(pTextureToBlur, &srvd, &pOrigShaderResourceView);

    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

    // ! I should probably reuse the textures and RenderTargetViews
    for (int i = 0; i <= iterations; i++)
    {
        ID3D11Texture2D* pBackBuffer;
        ID3D11RenderTargetView* mainRenderTargetView;
        ID3D11ShaderResourceView* pShaderResourceView;

        d3d11Device->CreateTexture2D(&desc, nullptr, &pBackBuffer);

        if (i == 0)
            mainRenderTargetView = pDstRenderTargetView;
        else
            d3d11Device->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
        d3d11Device->CreateShaderResourceView(pBackBuffer, nullptr, &pShaderResourceView);

        framebuffers.push_back(pBackBuffer);
        renderTargetViews.push_back(mainRenderTargetView);
        shaderResourceViews.push_back(pShaderResourceView);
        fbSizes.push_back(XMFLOAT2(desc.Width, desc.Height));

        desc.Width /= 2;
        desc.Height /= 2;
    }

    pTextureToBlur->GetDesc(&desc);

    constantBuffer.offset = XMFLOAT2(BLUR_OFFSET, BLUR_OFFSET);
    ppContext->PSSetShader(pDownsampleShader, nullptr, 0);
    RenderToRTV(renderTargetViews[1], pOrigShaderResourceView, fbSizes[1]);

    for (int i = 1; i < iterations; i++)
    {
        RenderToRTV(renderTargetViews[i + 1], shaderResourceViews[i], fbSizes[i + 1]);
    }

    ppContext->PSSetShader(pUpsampleShader, nullptr, 0);

    for (int i = iterations; i > 0; i--)
    {
        RenderToRTV(renderTargetViews[i - 1], shaderResourceViews[i], fbSizes[i - 1]);
    }

    ppContext->PSSetShader(dbgShader, nullptr, 0);
    RenderToRTV(pDstRenderTargetView, shaderResourceViews[0], fbSizes[0]);

    for (int i = 0; i < iterations; i++)
    {
        if (i != 0)
            renderTargetViews[i]->Release();
        framebuffers[i]->Release();
        shaderResourceViews[i]->Release();
    }
}

HRESULT DirectXHooks::IDXGISwapChain_Present(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags) {
    std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now() - start;
    frames += 1;

    if (elapsed.count() >= 0.5f) {
        fps = static_cast<int>((float)frames / elapsed.count());

        frames = 0;
        start = std::chrono::high_resolution_clock::now();
    }

    swapChain->GetDevice(IID_PPV_ARGS(&d3d11Device));
    d3d11Device->GetImmediateContext(&ppContext);
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    d3d11Device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);

    pBackBuffer->Release();

    if (!init) {
        //ImGui::CreateContext();
        //ImGui_ImplWin32_Init(FindWindowA(nullptr, "Minecraft: Windows 10 Edition"));
        //ImGui_ImplDX11_Init(d3d11Device, ppContext);
        init = true;
    }

    /*ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    ImGui::Begin("FPS");
    ImGui::Text("FPS: %.2f", fps);
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();*/

    // blur start

    static ID3D11Texture2D* pRenderedBuffer;

    if (!pRenderedBuffer)
    {
        D3D11_TEXTURE2D_DESC desc;
        pBackBuffer->GetDesc(&desc);
        desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        d3d11Device->CreateTexture2D(&desc, nullptr, &pRenderedBuffer);
    }
    ppContext->CopyResource(pRenderedBuffer, pBackBuffer);

    InitializePipeline();
    RenderBlur(pRenderedBuffer, mainRenderTargetView, 1);

    // blur end

    ppContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    //ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    mainRenderTargetView->Release();
    d3d11Device->Release();
    ppContext->Release();

    return PresentOriginal(swapChain, syncInterval, flags);
}

HRESULT DirectXHooks::IDXGISwapChain_ResizeBuffers(IDXGISwapChain3* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
    
    if (mainRenderTargetView)
    {
        mainRenderTargetView->Release();
        mainRenderTargetView = nullptr;
    }

    ID3D11Device* d3d11Device = nullptr;
    swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&d3d11Device);

    HRESULT hr = ResizeOriginal(swapChain, bufferCount, width, height, newFormat, swapChainFlags);

    if ((ResizeOriginal(swapChain, 0, width, height, newFormat, swapChainFlags)) && d3d11Device) {
        ID3D11Texture2D* pBackBuffer;
        D3D11_TEXTURE2D_DESC desc;
        swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        pBackBuffer->GetDesc(&desc);
        width = desc.Width;
        height = desc.Height;
        d3d11Device->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
        pBackBuffer->Release();
    }
    
    return hr;
}

void DirectXHooks::Init() {
    if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
        kiero::bind(8, (void**)&PresentOriginal, DirectXHooks::IDXGISwapChain_Present);
        kiero::bind(13, (void**)&ResizeOriginal, DirectXHooks::IDXGISwapChain_ResizeBuffers);
        return;
    }
}