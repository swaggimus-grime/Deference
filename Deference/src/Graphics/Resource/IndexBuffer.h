#pragma once

#include "Resource.h"
#include "Bindable.h"
#include "View.h"

namespace Def
{
	class IndexBuffer : public Resource, public Bindable, public SRV
	{
	public:
		IndexBuffer(Graphics& g, UINT numIndices, const UINT32* data);
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;
		virtual void Bind(Graphics& g) override;
		inline UINT NumIndices() const { return m_View.SizeInBytes / sizeof(UINT32); }

	private:
		D3D12_INDEX_BUFFER_VIEW m_View;
	};
}