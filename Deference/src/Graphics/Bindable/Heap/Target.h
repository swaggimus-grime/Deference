#pragma once

#include "Resource.h"

class Target : public Resource, public Bindable
{
public:
	virtual void Clear(Graphics& g) = 0;

	void CreateShaderResourceView(Graphics& g, HCPU hcpu);

protected:
	HCPU m_ShaderResourceHandle;

};