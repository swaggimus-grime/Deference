#pragma once

#include "Resource.h"

class Target : public Resource, public Bindable
{
public:
	Target();

	virtual void Clear(Graphics& g) = 0;

	void CreateShaderView(Graphics& g, HCPU h);
	virtual void Resize(Graphics& g, UINT w, UINT h) = 0;

protected:
	HCPU m_ShaderHCPU;

};