#include "UnorderedAccess.h"
#include "Graphics.h"

UnorderedAccess::UnorderedAccess(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle)
	:ShaderAccessible(handle, D3D12_RESOURCE_STATE_COPY_SOURCE)
{
    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.DepthOrArraySize = 1;
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    resDesc.Width = g.Width();
    resDesc.Height = g.Height();
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    g.Device().CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        m_State, nullptr,
        IID_PPV_ARGS(&m_Res));

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    g.Device().CreateUnorderedAccessView(m_Res.Get(), nullptr, &uavDesc,
        m_Handle);
}
