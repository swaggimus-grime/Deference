#pragma once

#include "Target.h"

class DepthStencil : public Target
{
public:
	DepthStencil(Graphics& g);

	virtual void CreateView(Graphics& g, HCPU h) override;
	virtual void Resize(Graphics& g, UINT w, UINT h) override;
	virtual void Clear(Graphics& g) override;
	virtual void Bind(Graphics& g) override;
};