#pragma once

#include "Event.h"

namespace Def
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(UINT w, UINT h)
			:m_Width(std::max(w, 1u)), m_Height(std::max(h, 1u))
		{}

		inline auto Width() const { return m_Width; }
		inline auto Height() const { return m_Height; }

	private:
		UINT m_Width;
		UINT m_Height;
	};
}