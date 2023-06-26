#include "RenderTarget.h"
#include "Debug/Exception.h"

RenderTarget::RenderTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES initState, ComPtr<ID3D12Resource> res)
    :Target(handle, initState, res)
{
    if (!res)
    {
        const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,
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
            &desc,
            m_State,
            &clearValue,
            IID_PPV_ARGS(&m_Res)
        );
    }

    g.Device().CreateRenderTargetView(m_Res.Get(), nullptr, m_Handle);
}

RenderTarget::RenderTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES initState)
    :RenderTarget(g, handle, initState, nullptr)
{
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

void RenderTarget::BindWithOther(Graphics& g, Target* ds)
{
    g.CL().OMSetRenderTargets(1, &m_Handle, false, &ds->Handle());
}
