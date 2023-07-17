#include "Swapchain.h"
#include "Graphics.h"

Swapchain::Swapchain(Graphics& g, HWND hWnd, UINT numBuffs)
    :m_NumBuffs(numBuffs), m_AllowTearing(true)
{
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &m_AllowTearing, sizeof(m_AllowTearing))))
            {
                m_AllowTearing = false;
            }
        }
    }

    ComPtr<IDXGISwapChain1> sc1;
    DXGI_SWAP_CHAIN_DESC1 scd;
    SecureZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC1));
    scd.Width = g.Width();
    scd.Height = g.Height();
    scd.Format = s_Format;
    scd.Stereo = FALSE;
    scd.SampleDesc = { 1, 0 };
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = m_NumBuffs;
    scd.Scaling = DXGI_SCALING_STRETCH;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    scd.Flags = m_AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    //scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    HR factory4->CreateSwapChainForHwnd(&g.CQ(), hWnd, &scd, nullptr, nullptr, &sc1);
    HR factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
    HR sc1.As(&m_SC);

    m_RTs = MakeUnique<RenderTargetHeap>(g, m_NumBuffs);
    for (int i = 0; i < m_NumBuffs; ++i)
    {
        ComPtr<ID3D12Resource> bb;
        HR m_SC->GetBuffer(i, IID_PPV_ARGS(&bb));
        m_BackBuffs.push_back(std::move(m_RTs->Add(g, D3D12_RESOURCE_STATE_PRESENT, std::move(bb))));
    }
}

void Swapchain::Present()
{
    m_SC->Present(0, m_AllowTearing ? DXGI_FEATURE_PRESENT_ALLOW_TEARING : 0);
}
