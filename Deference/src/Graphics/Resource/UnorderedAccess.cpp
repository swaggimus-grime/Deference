#include "UnorderedAccess.h"
#include "Graphics.h"

namespace Def
{
    UnorderedAccess::UnorderedAccess(Graphics& g, DXGI_FORMAT format)
        :UAV(this)
    {
        m_State = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        D3D12_RESOURCE_DESC desc = {};
        desc.DepthOrArraySize = 1;
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Format = format;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        desc.Width = g.Width();
        desc.Height = g.Height();
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        HR g.Device().CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
            IID_PPV_ARGS(&m_Res));
    }

    const D3D12_UNORDERED_ACCESS_VIEW_DESC& UnorderedAccess::UAVDesc() const
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
        desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        return desc;
    }

    void UnorderedAccess::Resize(Graphics& g, UINT w, UINT h)
    {
        D3D12_RESOURCE_DESC desc = {};
        desc.DepthOrArraySize = 1;
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Format = GetFormat();
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        desc.Width = w;
        desc.Height = h;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        HR g.Device().CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
            IID_PPV_ARGS(&m_Res));

        if (UAVHCPU().ptr != 0)
            UAV::CreateView(g, UAVHCPU());

        m_State = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
}