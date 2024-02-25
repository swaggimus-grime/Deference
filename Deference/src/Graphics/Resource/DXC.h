#pragma once

namespace Def
{
	namespace DXC
	{
		void Compile(const std::wstring& path, const std::wstring& type, ComPtr<IDxcBlob>& byteCode);
		void Compile(const std::wstring& path, ComPtr<IDxcBlob>& byteCode);
	}
}