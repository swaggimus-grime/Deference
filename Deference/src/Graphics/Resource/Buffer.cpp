#include "Buffer.h"
#include "Debug/Exception.h"

namespace Def
{
	const D3D12_HEAP_PROPERTIES UPLOAD_HEAP_PROPS = {
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0,
	};

	const D3D12_HEAP_PROPERTIES DEFAULT_HEAP_PROPS = {
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0,
	};

	const D3D12_HEAP_PROPERTIES READBACK_HEAP_PROPS = {
		D3D12_HEAP_TYPE_READBACK,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0,
	};

	const D3D12_RESOURCE_DESC& RawBuffer::Desc(SIZE_T bytes, D3D12_RESOURCE_FLAGS flags)
	{
		CD3DX12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Buffer({ D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT , bytes });
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = bytes;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.SampleDesc = { 1, 0 };
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Flags = flags;
		return std::move(desc);
	}

	RawBuffer::RawBuffer(Graphics& g, D3D12_HEAP_TYPE type, SIZE_T numBytes,
		D3D12_RESOURCE_STATES state,
		D3D12_RESOURCE_FLAGS flags)
		:SRV(this), m_Size(numBytes)
	{
		m_State = state;

		auto props = CD3DX12_HEAP_PROPERTIES(type);
		auto desc = Desc(numBytes, flags);
		HR g.Device().CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, state, nullptr, IID_PPV_ARGS(&m_Res));
	}

	RawBuffer::RawBuffer(Graphics& g, void* initData, SIZE_T numBytes)
		:RawBuffer(g, D3D12_HEAP_TYPE_DEFAULT, numBytes, D3D12_RESOURCE_STATE_COMMON)
	{
		auto props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto desc = Desc(numBytes, D3D12_RESOURCE_FLAG_NONE);
		ComPtr<ID3D12Resource> upload;
		HR g.Device().CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload));

		// Indicate what data have to be copied to the upload buffer
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = initData;
		subResourceData.RowPitch = numBytes;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		Transition(g, D3D12_RESOURCE_STATE_COPY_DEST);
		UpdateSubresources<1>(&g.CL(), m_Res.Get(), upload.Get(), 0, 0, 1, &subResourceData);
		Transition(g, D3D12_RESOURCE_STATE_GENERIC_READ);

		g.Flush();
	}

	const D3D12_SHADER_RESOURCE_VIEW_DESC& RawBuffer::SRVDesc() const
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Buffer.FirstElement = 0;
		srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		return std::move(srv_desc);
	}

	StructuredBuffer::StructuredBuffer(Graphics& g, D3D12_HEAP_TYPE type, SIZE_T numElements, SIZE_T stride, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_FLAGS flags)
		:RawBuffer(g, type, numElements * stride, state, flags), m_NumElements(numElements), m_Stride(stride)
	{
	}

	const D3D12_SHADER_RESOURCE_VIEW_DESC& StructuredBuffer::SRVDesc() const
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Buffer.NumElements = m_NumElements;
		srv_desc.Buffer.StructureByteStride = m_Stride;
		srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		return std::move(srv_desc);
	}

	void* RawBuffer::Map()
	{
		void* mapping = nullptr;
		D3D12_RANGE range;
		range.Begin = 0;
		range.End = 0;
		HR m_Res->Map(0, &range, &mapping);
		return mapping;
	}

	void* RawBuffer::Map(D3D12_RANGE range)
	{
		void* mapping = nullptr;
		HR m_Res->Map(0, &range, &mapping);
		return mapping;
	}

	void RawBuffer::Unmap()
	{
		m_Res->Unmap(0, nullptr);
	}

	void RawBuffer::Unmap(D3D12_RANGE written)
	{
		m_Res->Unmap(0, &written);
	}

	ByteBuffer::ByteBuffer(Graphics& g, D3D12_HEAP_TYPE type, SIZE_T numElements, SIZE_T stride, D3D12_RESOURCE_STATES state)
		:StructuredBuffer(g, type, numElements, stride, state)
	{
	}

	const D3D12_SHADER_RESOURCE_VIEW_DESC& ByteBuffer::SRVDesc() const
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = DXGI_FORMAT_R32_TYPELESS; //DXGI_FORMAT_ConvertToTypeless(m_Format);
		desc.Buffer.NumElements = m_NumElements / 2;
		desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		desc.Buffer.StructureByteStride = 0;
		desc.Buffer.FirstElement = 0;

		return std::move(desc);
	}
}
