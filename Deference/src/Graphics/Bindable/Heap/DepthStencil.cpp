#include "DepthStencil.h"

DepthStencil::DepthStencil(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle)
    :Target(handle, nullptr, D3D12_RESOURCE_STATE_DEPTH_WRITE)
{
    D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    desc.Flags = D3D12_DSV_FLAG_NONE;

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
        m_State,
        &clear,
        IID_PPV_ARGS(&m_Res)
    );
    
    g.Device().CreateDepthStencilView(m_Res.Get(), &desc, m_Handle);
}

void DepthStencil::Clear(Graphics& g)
{
	g.CL().ClearDepthStencilView(m_Handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DepthStencil::Bind(Graphics& g)
{
	g.CL().OMSetRenderTargets(0, nullptr, false, &m_Handle);
}

void DepthStencil::BindWithOther(Graphics& g, Target* rt)
{
	rt->BindWithOther(g, this);
}
