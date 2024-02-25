#pragma once

#include "Target.h"

namespace Def
{
	class DepthStencil : public Target, public DSV
	{
	public:
		DepthStencil(Graphics& g);
		virtual const D3D12_DEPTH_STENCIL_VIEW_DESC& DSVDesc() const override;
		virtual void Clear(Graphics& g) const override;
		virtual void Bind(Graphics& g) override;
		virtual void Resize(Graphics& g, UINT w, UINT h) override;
	};
}