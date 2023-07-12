#include "RenderTarget.h"
#include "Debug/Exception.h"

RenderTarget::RenderTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES initState)
    :Target(handle, nullptr, initState)
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
        m_State,
        &clearValue,
        IID_PPV_ARGS(&m_Res)
    );

    g.Device().CreateRenderTargetView(m_Res.Get(), nullptr, m_Handle);
}

RenderTarget::RenderTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES initState, ComPtr<ID3D12Resource> res)
    :Target(handle, res, initState)
{
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

void RenderTarget::BindWithOther(Graphics& g, Target* ds)
{
    g.CL().OMSetRenderTargets(1, &m_Handle, false, &ds->Handle());
}

RTV::RTV(Graphics& g, const D3D12_CPU_DESCRIPTOR_HANDLE& handle, Shared<RenderTarget> rt)
    :Resource(handle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = rt->Res()->GetDesc().Format;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Texture2D.MipLevels = 1;
    desc.Texture2D.MostDetailedMip = 0;
    desc.Texture2D.PlaneSlice = 0;

    g.Device().CreateShaderResourceView(rt->Res(), &desc, m_Handle);
}

ReadbackRenderTarget::ReadbackRenderTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle)
    :RenderTarget(g, handle, D3D12_RESOURCE_STATE_COPY_DEST)
{
}

CopyTarget::CopyTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle)
    :Resource(handle)
{
    const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const D3D12_RESOURCE_DESC res = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,
        static_cast<UINT64>(g.Width()),
        static_cast<UINT>(g.Height()),
        1, 1, 1, 0);

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    XMFLOAT4 col = { 0.f, 0.f, 0.f, 1.f };
    std::memcpy(clearValue.Color, &col, sizeof(XMFLOAT4));

    HR g.Device().CreateCommittedResource(
        &heap,
        D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
        &res,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&m_Res)
    );
}
