#pragma once

#include "Resource.h"

class Target : public Resource, public Bindable
{
public:
	Target()
	{
		m_ShaderResourceHandle.ptr = 0;
	}

	virtual void Clear(Graphics& g) = 0;

	void CreateShaderResourceView(Graphics& g, HCPU hcpu);
	virtual void Resize(Graphics& g, UINT w, UINT h) = 0;
protected:
	HCPU m_ShaderResourceHandle;

};