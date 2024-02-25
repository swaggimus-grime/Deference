#pragma once

#include "BufferElement.h"
#include "InputLayout.h"
#include "Debug/Exception.h"
#include "View.h"
#include "Bindable.h"
#include "Resource.h"

namespace Def
{
	class VertexStream
	{
	public:
		using enum VERTEX_ATTRIBUTES;

		VertexStream(const InputLayout& layout, UINT numVertices);

		inline BufferElement operator()(const std::string& attr, UINT idx)
		{
			return BufferElement(m_Data.data() + idx * m_Stride + m_Offsets[attr]);
		}

		inline const void* Data() const { return m_Data.data(); }
		inline UINT Size() const { return m_Stride * m_NumVertices; }
		inline UINT Stride() const { return m_Stride; }

	private:
		std::vector<CHAR> m_Data;
		UINT m_NumVertices;
		UINT m_Stride;
		std::unordered_map<std::string, SIZE_T> m_Offsets;
	};

	class VertexBuffer : public Resource, public Bindable, public SRV
	{
	public:
		VertexBuffer(Graphics& g, const VertexStream& stream);
		virtual void Bind(Graphics& g) override;
		inline UINT NumVertices() const { return m_View.SizeInBytes / Stride(); }
		inline UINT Stride() const { return m_View.StrideInBytes; }
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;

	private:
		D3D12_VERTEX_BUFFER_VIEW m_View;
	};
}