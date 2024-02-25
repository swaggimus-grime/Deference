#pragma once

#include "Resource.h"
#include "View.h"

namespace Def
{
	class Surface : public Resource
	{
	public:
		inline UINT GetWidth() const { return m_Res->GetDesc().Width; }
		inline UINT GetHeight() const { return m_Res->GetDesc().Height; }
		inline DXGI_FORMAT GetFormat() const { return m_Res->GetDesc().Format; }
	};
}