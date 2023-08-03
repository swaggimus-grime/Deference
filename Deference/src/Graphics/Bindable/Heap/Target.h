#pragma once

#include "Resource.h"

class Target : public Resource, public Bindable
{
public:
	Target()
	{
		m_ShaderResourceHandle.m_HCPU.ptr = 0;
		m_ShaderResourceHandle.m_HGPU.ptr = 0;
	}

	virtual void Clear(Graphics& g) = 0;

	void CreateShaderResourceView(Graphics& g, HDESC h);
	virtual void Resize(Graphics& g, UINT w, UINT h) = 0;

	inline HGPU GetShaderHGPU() const { return m_ShaderResourceHandle.m_HGPU; }

protected:
	HDESC m_ShaderResourceHandle;

};