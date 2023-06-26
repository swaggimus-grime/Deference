#include "VertexBuffer.h"

VertexStream::VertexStream(const InputLayout& layout, UINT numVertices)
    :m_Offsets(layout.m_Offsets), m_Stride(layout.m_Stride), m_NumVertices(numVertices)
{
    m_Data.reserve(m_NumVertices * m_Stride);
}

VertexBuffer::VertexBuffer(Graphics& g, const VertexStream& stream)
{
    g.CreateBuffer(m_Buffer, stream.Size(), stream.Data(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_Buffer->SetName(L"Vertex Buffer");
    m_View = {
        .BufferLocation = m_Buffer->GetGPUVirtualAddress(),
        .SizeInBytes = stream.Size(),
        .StrideInBytes = stream.Stride()
    };
}

void VertexBuffer::Bind(Graphics& g)
{
    g.CL().IASetVertexBuffers(0, 1, &m_View);
}