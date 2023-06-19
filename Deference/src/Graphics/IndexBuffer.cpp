#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(Graphics& g, UINT numIndices, const UINT32* data)
{
    g.CreateBuffer(m_Buffer, sizeof(UINT32) * numIndices, data, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    m_View = {
        .BufferLocation = m_Buffer->GetGPUVirtualAddress(),
        .SizeInBytes = static_cast<UINT>(sizeof(UINT32)) * numIndices,
        .Format = DXGI_FORMAT_R32_UINT
    };
}

void IndexBuffer::Bind(Graphics& g) const
{
    g.CL().IASetIndexBuffer(&m_View);
}