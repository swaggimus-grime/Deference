#pragma once

#include "Target.h"
#include <d3d12.h>

namespace Def
{
	class Graphics;

	class UnorderedAccess : public Surface, public Resizeable, public UAV
	{
	public:
		UnorderedAccess(Graphics& g, DXGI_FORMAT format);
		virtual const D3D12_UNORDERED_ACCESS_VIEW_DESC& UAVDesc() const override;
		virtual void Resize(Graphics& g, UINT w, UINT h) override;
	};
}