#pragma once

#include "Surface.h"
#include "Resizeable.h"
#include "Bindable.h"

namespace Def
{
	class Target : public Surface, public Bindable, public Resizeable, public SRV
	{
	public:
		virtual void Clear(Graphics& g) const = 0;
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;

	protected:
		Target();
	};
}