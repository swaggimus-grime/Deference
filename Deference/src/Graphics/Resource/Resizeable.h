#pragma once

namespace Def
{
	class Resizeable
	{
	public:
		virtual void Resize(Graphics& g, UINT w, UINT h) = 0;

	protected:
		Resizeable() = default;
	};
}