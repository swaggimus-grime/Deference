#pragma once

#include "Resource.h"
#include "Bindable/Bindable.h"
#include <d3d12.h>
#include "Graphics.h"

template<typename Params>
class ConstantBuffer : public Resource
{
public:
    virtual void Update(Graphics& g) {}

protected:
    void UpdateStruct(Params* pData)
    {
        uint8_t* data;
        HR m_Res->Map(0, nullptr, reinterpret_cast<void**>(&data));
        std::memcpy(data, pData, m_Size);
        m_Res->Unmap(0, nullptr);
    }

    ConstantBuffer(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle)
        :Resource(handle, nullptr, D3D12_RESOURCE_STATE_GENERIC_READ),
        m_Size(ALIGN(sizeof(Params), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
    {
        g.CreateBuffer(m_Res, m_Size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
        desc.BufferLocation = m_Res->GetGPUVirtualAddress();
        desc.SizeInBytes = m_Size;
        g.Device().CreateConstantBufferView(&desc, m_Handle);
    }

private:
    UINT m_Size;
};