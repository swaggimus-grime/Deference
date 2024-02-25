#pragma once

#include "Surface.h"
#include "Graphics.h"

namespace Def
{
	class Texture2D : public Surface, public SRV
	{
	public:
		Texture2D(Graphics& g, UINT8* start, SIZE_T byteLength);
		Texture2D(Graphics& g, const std::wstring& path);
		Texture2D(Graphics& g, const std::string& path);
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;

	private:
		void CreateFromFile(Graphics& g, const std::wstring& path);
		void CreateFromMemory(Graphics& g, UINT8* start, SIZE_T byteLength);
	};

	class EnvironmentMap : public Surface, public SRV
	{
	public:
		EnvironmentMap(Graphics& g, const std::wstring& path);
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;
	};

	class CubeMap : public Resource, public SRV
	{
	public:
		CubeMap(Graphics& g, const std::wstring& path);
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;
	};
}