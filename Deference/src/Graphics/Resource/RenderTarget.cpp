#include "RenderTarget.h"
#include "Debug/Exception.h"

namespace Def
{
    RenderTarget::RenderTarget(Graphics& g, DXGI_FORMAT fmt, XMFLOAT4 clearColor)
        :RTV(this), m_ClearColor(std::move(clearColor))
    {
        m_State = D3D12_RESOURCE_STATE_RENDER_TARGET;

        const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const D3D12_RESOURCE_DESC res = CD3DX12_RESOURCE_DESC::Tex2D(fmt,
            static_cast<UINT64>(g.Width()),
            static_cast<UINT>(g.Height()),
            1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = fmt;
        std::memcpy(clearValue.Color, &m_ClearColor, sizeof(XMFLOAT4));

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
        :RTV(this)
    {
        m_State = D3D12_RESOURCE_STATE_PRESENT;
        m_Res = res;
    }

    const D3D12_RENDER_TARGET_VIEW_DESC& RenderTarget::RTVDesc() const
    {
        return D3D12_RENDER_TARGET_VIEW_DESC();
    }

    void RenderTarget::Clear(Graphics& g) const
    {
        g.CL().ClearRenderTargetView(RTVHCPU(), reinterpret_cast<const FLOAT*>(&m_ClearColor), 0, nullptr);
    }

    void RenderTarget::Bind(Graphics& g)
    {
        g.CL().OMSetRenderTargets(1, &RTVHCPU(), false, nullptr);
    }

    void RenderTarget::BindWithDepth(Graphics& g, DepthStencil* ds)
    {
        g.CL().OMSetRenderTargets(1, &RTVHCPU(), false, &ds->DSVHCPU());
    }

    void RenderTarget::Resize(Graphics& g, UINT w, UINT h)
    {
        m_State = D3D12_RESOURCE_STATE_RENDER_TARGET;

        const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const D3D12_RESOURCE_DESC res = CD3DX12_RESOURCE_DESC::Tex2D(GetFormat(),
            static_cast<UINT64>(w),
            static_cast<UINT>(h),
            1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = GetFormat();
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

        if (RTVHCPU().ptr != 0)
            RTV::CreateView(g, RTVHCPU());
        
        if (SRVHCPU().ptr != 0)
            SRV::CreateView(g, SRVHCPU());
    }
}