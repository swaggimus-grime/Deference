#pragma once

#include "Bindable.h"

class Graphics;

class Viewport : public Bindable
{
public:
	Viewport(Graphics& g);
	virtual void Bind(Graphics& g) override;
	void Resize(UINT w, UINT h);

private:
	D3D12_VIEWPORT m_VP;
	D3D12_RECT m_SR;
};