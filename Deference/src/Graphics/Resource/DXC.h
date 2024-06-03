#pragma once

namespace Def
{
	namespace DXC
	{
		ComPtr<IDxcBlob> Compile(std::wstring_view path);
	}
}