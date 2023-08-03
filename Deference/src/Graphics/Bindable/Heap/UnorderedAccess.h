#pragma once

#include "Resource.h"
#include <d3d12.h>

class Graphics;

class UnorderedAccess : public Resource
{
public:
	UnorderedAccess(Graphics& g);
	void CreateView(Graphics& g, HDESC h);
	void Resize(Graphics& g, UINT w, UINT h);

};