#include "RenderTarget.h"
#include "Debug/Exception.h"

RenderTarget::RenderTarget(Graphics& g)
{
    const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const D3D12_RESOURCE_DESC res = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,
        static_cast<UINT64>(g.Width()),
        static_cast<UINT>(g.Height()),
        1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    XMFLOAT4 col = { 0.f, 0.f, 0.f, 1.f };
    std::memcpy(clearValue.Color, &col, sizeof(XMFLOAT4));

    HR g.Device().CreateCommittedResource(
        &heap,
        D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
        &res,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearValue,
        IID_PPV_ARGS(&m_Res)
    );
}

RenderTarget::RenderTarget(const ComPtr<ID3D12Resource>& res)
{
    m_Res = std::move(res);
}

void RenderTarget::CreateView(Graphics& g, HCPU hcpu)
{
    m_Handle = hcpu;
    g.Device().CreateRenderTargetView(m_Res.Get(), nullptr, m_Handle);
}

void RenderTarget::Clear(Graphics& g)
{
   FLOAT col[] = { 0.f, 0.f, 0.f, 1.f };
   g.CL().ClearRenderTargetView(m_Handle, col, 0, nullptr);
}

void RenderTarget::Bind(Graphics& g)
{
    g.CL().OMSetRenderTargets(1, &m_Handle, false, nullptr);
}

void RenderTarget::BindWithDepth(Graphics& g, Shared<DepthStencil> ds)
{
    BindWithDepth(g, *ds);
}

void RenderTarget::BindWithDepth(Graphics& g, DepthStencil& ds)
{
    auto depth = ds.GetView();
    g.CL().OMSetRenderTargets(1, &m_Handle, false, &depth);
}
