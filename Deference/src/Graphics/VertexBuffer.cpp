#include "VertexBuffer.h"

#define CHECK_ATTRIB(x) if ((attributes & x) == x) { \
                            m_Elements.push_back({Map<x>::name, 0, Map<x>::format, 0, m_Stride, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}); \
                            m_Offsets.insert({ x, m_Stride }); \
                            m_Stride += sizeof(Map<x>::type); \
                        } \

InputLayout::InputLayout(const VERTEX_ATTRIBUTES& attributes)
    :m_Attributes(attributes), m_Stride(0u)
{
    using enum VERTEX_ATTRIBUTES;
    CHECK_ATTRIB(POS)
    CHECK_ATTRIB(TEX)
    CHECK_ATTRIB(NORM)
    CHECK_ATTRIB(TAN)
    CHECK_ATTRIB(BITAN)
    CHECK_ATTRIB(COLOR)

    m_Layout.NumElements = m_Elements.size();
    m_Layout.pInputElementDescs = m_Elements.data();
}


VertexStream::VertexStream(const InputLayout& layout, UINT numVertices)
    :m_Offsets(layout.m_Offsets), m_Stride(layout.m_Stride), m_NumVertices(numVertices)
{
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
}

void VertexBuffer::Bind(Graphics& g) const
{
    g.CL().IASetVertexBuffers(0, 1, &m_View);
}