#pragma once

#include "Resource.h"

namespace Def
{
	class GenericBuffer : public Resource, public SRV
	{
	public:
		GenericBuffer(Graphics& g, D3D12_HEAP_TYPE type, SIZE_T numBytes,
			D3D12_RESOURCE_STATES state,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
		
		GenericBuffer(Graphics& g, void* initData, SIZE_T numBytes);

		void* Map();
		void* Map(D3D12_RANGE range);
		void Unmap();
		void Unmap(D3D12_RANGE written);
		inline SIZE_T Size() const { return m_Size; }

		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;

	protected:
		static const D3D12_RESOURCE_DESC& Desc(SIZE_T bytes, D3D12_RESOURCE_FLAGS flags);

	private:
		SIZE_T m_Size;
	};

	class StructuredBuffer : public GenericBuffer
	{
	public:
		StructuredBuffer(Graphics& g, D3D12_HEAP_TYPE type, SIZE_T numElements, SIZE_T stride, D3D12_RESOURCE_STATES state,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
		inline SIZE_T NumElements() const { return m_NumElements; }
		inline SIZE_T GetStride() const { return m_Stride; }
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;

	protected:
		SIZE_T m_NumElements;
		SIZE_T m_Stride;
	};

	class RawBuffer : public StructuredBuffer
	{
	public:
		RawBuffer(Graphics& g, D3D12_HEAP_TYPE type, SIZE_T numElements, SIZE_T stride, D3D12_RESOURCE_STATES state);
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;

	};
}