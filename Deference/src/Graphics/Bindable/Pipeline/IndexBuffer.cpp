#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(Graphics& g, UINT numIndices, const UINT32* data)
{
    g.CreateBuffer(m_Res, sizeof(UINT32) * numIndices, data, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    m_Res->SetName(L"Index Buffer");
    m_View = {
        .BufferLocation = m_Res->GetGPUVirtualAddress(),
        .SizeInBytes = static_cast<UINT>(sizeof(UINT32)) * numIndices,
        .Format = DXGI_FORMAT_R32_UINT
    };
}

void IndexBuffer::Bind(Graphics& g)
{
    g.CL().IASetIndexBuffer(&m_View);
}

void IndexBuffer::CreateView(Graphics& g, HDESC h)
{
	SetHandle(h);
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.Buffer.NumElements = NumIndices();
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	desc.Buffer.StructureByteStride = 0;

	g.Device().CreateShaderResourceView(m_Res.Get(), &desc, GetHCPU());
}
