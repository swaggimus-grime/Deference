#include "ConstantBuffer.h"
#include "Graphics.h"

ConstantBuffer::ConstantBuffer(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res, SIZE_T size)
    :Resource(handle, D3D12_RESOURCE_STATE_GENERIC_READ, res)
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
    desc.BufferLocation = m_Res->GetGPUVirtualAddress();
    desc.SizeInBytes = size;
    g.Device().CreateConstantBufferView(&desc, m_Handle);
}