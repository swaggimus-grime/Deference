#pragma once

#include "Resource.h"
#include <d3d12.h>

class Graphics;

class UnorderedAccess : public Resource
{
public:
	UnorderedAccess(Graphics& g, DXGI_FORMAT format);
	void CreateView(Graphics& g, HCPU h);
	void Resize(Graphics& g, UINT w, UINT h);

private:
	DXGI_FORMAT m_Format;
};