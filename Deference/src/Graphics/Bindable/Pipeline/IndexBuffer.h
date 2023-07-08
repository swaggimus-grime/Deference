#pragma once

#include "Bindable/Bindable.h"

class IndexBuffer : public Bindable
{
public:
	IndexBuffer(Graphics& g, UINT numIndices, const UINT32* data);
	virtual void Bind(Graphics& g) override;
	inline auto* Res() const { return m_Buffer.Get(); }
	inline UINT NumIndices() const { return m_View.SizeInBytes / sizeof(UINT32); }

private:
	ComPtr<ID3D12Resource> m_Buffer;
	D3D12_INDEX_BUFFER_VIEW m_View;
};