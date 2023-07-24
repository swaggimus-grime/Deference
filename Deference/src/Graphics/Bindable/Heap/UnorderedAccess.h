#pragma once

#include "Resource.h"
#include <d3d12.h>

class Graphics;

class UnorderedAccess : public Resource
{
public:
	UnorderedAccess(Graphics& g);
	void CreateView(Graphics& g, HCPU hcpu);

private:
	HCPU m_Handle;
};