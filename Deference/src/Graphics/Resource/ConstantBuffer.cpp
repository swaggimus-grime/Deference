#include "ConstantBuffer.h"
#include "Debug/Exception.h"
#include "Graphics.h"

namespace Def
{
	ConstantBuffer::ConstantBuffer(Graphics& g, const ConstantBufferLayout& layout)
		:CBV(this), m_Constants(std::move(layout.m_Constants))
	{
		m_State = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		g.CreateBuffer(m_Res, ALIGN(layout.m_Size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT),
			D3D12_HEAP_TYPE_UPLOAD, m_State);

		BeginUpdate();
	}

	const D3D12_CONSTANT_BUFFER_VIEW_DESC& ConstantBuffer::CBVDesc() const
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
		desc.BufferLocation = GetGPUAddress();
		desc.SizeInBytes = m_Res->GetDesc().Width;

		return std::move(desc);
	}

	void ConstantBuffer::BeginUpdate()
	{
		HR m_Res->Map(0, nullptr, reinterpret_cast<void**>(&m_Data));
	}

	void ConstantBuffer::EndUpdate()
	{
		m_Res->Unmap(0, nullptr);
	}
}