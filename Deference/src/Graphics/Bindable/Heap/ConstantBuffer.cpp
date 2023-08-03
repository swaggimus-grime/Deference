#include "ConstantBuffer.h"
#include "Debug/Exception.h"
#include "Graphics.h"

ConstantBuffer::ConstantBuffer(Graphics& g, const ConstantBufferLayout& layout) 
	:m_Constants(std::move(layout.m_Constants))
{
	g.CreateBuffer(m_Res, ALIGN(layout.m_Size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), 
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	BeginUpdate();
}

void ConstantBuffer::CreateView(Graphics& g, HDESC h)
{
	SetHandle(h);
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = m_Res->GetGPUVirtualAddress();
	desc.SizeInBytes = m_Res->GetDesc().Width;

	g.Device().CreateConstantBufferView(&desc, GetHCPU());
}

void ConstantBuffer::BeginUpdate()
{
	HR m_Res->Map(0, nullptr, reinterpret_cast<void**>(&m_Data));
}

void ConstantBuffer::EndUpdate()
{
	m_Res->Unmap(0, nullptr);
}