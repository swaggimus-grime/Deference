#pragma once

#include "Resource.h"

class Target : public Resource, public Bindable
{
public:
	Target();

	virtual void Clear(Graphics& g) = 0;

	void CreateShaderResourceView(Graphics& g, HDESC h);
	virtual void Resize(Graphics& g, UINT w, UINT h) = 0;
	void CopyShaderResourceView(Graphics& g, HCPU h);

protected:
	HCPU m_ShaderResourceHCPU;

};