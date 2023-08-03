#pragma once

#include "Bindable/Bindable.h"

class IndexBuffer : public Resource, public Bindable
{
public:
	IndexBuffer(Graphics& g, UINT numIndices, const UINT32* data);
	virtual void Bind(Graphics& g) override;
	virtual void CreateView(Graphics& g, HDESC h) override;
	inline UINT NumIndices() const { return m_View.SizeInBytes / sizeof(UINT32); }

private:
	D3D12_INDEX_BUFFER_VIEW m_View;
};