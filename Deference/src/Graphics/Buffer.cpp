#include "pch.h"
#include "Buffer.h"

#define CHECK_ATTRIB(x) if ((attributes & x) == x) { \
                            m_Elements.push_back({Map<x>::name, 0, Map<x>::format, 0, m_Stride, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}); \
                            m_Offsets.insert({ x, m_Stride }); \
                            m_Stride += sizeof(Map<x>::type); \
                        } \

VertexStream::VertexStream(const VERTEX_ATTRIBUTES& attributes, UINT numVertices)
    :m_Attributes(attributes), m_Stride(0), m_NumVertices(numVertices)
{
    CHECK_ATTRIB(POS)
    CHECK_ATTRIB(TEX)
    CHECK_ATTRIB(NORM)
    CHECK_ATTRIB(TAN)
    CHECK_ATTRIB(BITAN)
    CHECK_ATTRIB(COLOR)

    m_Data.reserve(m_NumVertices * m_Stride);
}

VertexBuffer::VertexBuffer(Graphics& g, const VertexStream& stream)
{
    g.CreateBuffer(m_Buffer, stream.Size(), stream.Data(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_View = {
        .BufferLocation = m_Buffer->GetGPUVirtualAddress(),
        .SizeInBytes = stream.Size(),
        .StrideInBytes = stream.Stride()
    };

    m_Layout.NumElements = stream.NumElements();
    m_Layout.pInputElementDescs = stream.Elements();
}

void VertexBuffer::Bind(Graphics& g) const
{
    g.CL().IASetVertexBuffers(0, 1, &m_View);
}

IndexBuffer::IndexBuffer(Graphics& g, UINT numIndices, const UINT16* data)
{
    g.CreateBuffer(m_Buffer, sizeof(UINT16) * numIndices, data, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    m_View = {
        .BufferLocation = m_Buffer->GetGPUVirtualAddress(),
        .SizeInBytes = static_cast<UINT>(sizeof(UINT16)) * numIndices,
        .Format = DXGI_FORMAT_R16_UINT
    };
}

void IndexBuffer::Bind(Graphics& g) const
{
    g.CL().IASetIndexBuffer(&m_View);
}
