#include "DepthStencil.h"

namespace Def
{
    DepthStencil::DepthStencil(Graphics& g)
        :DSV(this)
    {
        m_State = D3D12_RESOURCE_STATE_DEPTH_WRITE;

        D3D12_CLEAR_VALUE clear = {};
        clear.Format = DXGI_FORMAT_D32_FLOAT;
        clear.DepthStencil.Depth = 1.0f;
        clear.DepthStencil.Stencil = 0;

        const CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);
        const auto res = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, g.Width(), g.Height(), 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        HR g.Device().CreateCommittedResource(
            &heap,
            D3D12_HEAP_FLAG_NONE,
            &res,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clear,
            IID_PPV_ARGS(&m_Res)
        );
    }

    void DepthStencil::Resize(Graphics& g, UINT w, UINT h)
    {
        D3D12_CLEAR_VALUE clear = {};
        clear.Format = DXGI_FORMAT_D32_FLOAT;
        clear.DepthStencil.Depth = 1.0f;
        clear.DepthStencil.Stencil = 0;

        const CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);
        const auto res = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, w, h, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        HR g.Device().CreateCommittedResource(
            &heap,
            D3D12_HEAP_FLAG_NONE,
            &res,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clear,
            IID_PPV_ARGS(&m_Res)
        );

        if (DSVHCPU().ptr != 0)
            DSV::CreateView(g, DSVHCPU());

        if (SRVHCPU().ptr != 0)
            SRV::CreateView(g, SRVHCPU());
    }

    void DepthStencil::Clear(Graphics& g) const
    {
        g.CL().ClearDepthStencilView(DSVHCPU(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    void DepthStencil::Bind(Graphics& g)
    {
        g.CL().OMSetRenderTargets(0, nullptr, false, &DSVHCPU());
    }

    const D3D12_DEPTH_STENCIL_VIEW_DESC& DepthStencil::DSVDesc() const
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        desc.Format = GetFormat();
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        desc.Flags = D3D12_DSV_FLAG_NONE;

        return std::move(desc);
    }
}