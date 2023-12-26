#pragma once

#include "Resource.h"

class Target : public Resource, public Bindable
{
public:
	Target();

	virtual void Clear(Graphics& g) = 0;
	inline DXGI_FORMAT GetFormat() const { return m_Res->GetDesc().Format; }
	void CreateShaderView(Graphics& g, HCPU h);
	virtual void Resize(Graphics& g, UINT w, UINT h) = 0;

protected:
	HCPU m_ShaderHCPU;
	DXGI_FORMAT m_Format;
};