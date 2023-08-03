#include "VertexBuffer.h"

VertexStream::VertexStream(const InputLayout& layout, UINT numVertices)
    :m_Offsets(layout.m_Offsets), m_Stride(layout.m_Stride), m_NumVertices(numVertices)
{
    m_Data.reserve(m_NumVertices * m_Stride);
}

VertexBuffer::VertexBuffer(Graphics& g, const VertexStream& stream)
{
    g.CreateBuffer(m_Res, stream.Size(), stream.Data(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_Res->SetName(L"Vertex Buffer");
    m_View = {
        .BufferLocation = m_Res->GetGPUVirtualAddress(),
        .SizeInBytes = stream.Size(),
        .StrideInBytes = stream.Stride()
    };
}

void VertexBuffer::Bind(Graphics& g)
{
    g.CL().IASetVertexBuffers(0, 1, &m_View);
}

void VertexBuffer::CreateView(Graphics& g, HDESC h)
{
    SetHandle(h);
    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Buffer.NumElements = NumVertices();
    desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    desc.Buffer.StructureByteStride = Stride();
    g.Device().CreateShaderResourceView(m_Res.Get(), &desc, GetHCPU());
}
