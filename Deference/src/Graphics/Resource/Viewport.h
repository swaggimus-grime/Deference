#pragma once

#include "Bindable.h"
#include "Resizeable.h"

namespace Def
{
	class Graphics;

	class Viewport : public Bindable, public Resizeable
	{
	public:
		Viewport(UINT w, UINT h);
		virtual void Bind(Graphics& g) override;
		virtual void Resize(Graphics& g, UINT w, UINT h) override;

	private:
		D3D12_VIEWPORT m_VP;
		D3D12_RECT m_SR;
	};
}