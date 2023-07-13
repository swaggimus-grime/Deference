#pragma once

#include "Resource.h"
#include <DirectXTex/DirectXTex.h>

class Texture : public Resource
{
protected:
	Texture(const D3D12_CPU_DESCRIPTOR_HANDLE& handle);
};

class Texture2D : public Texture
{
public:
	Texture2D(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, const std::wstring& path);
};

class EnvironmentMap : public Texture
{
public:
	EnvironmentMap(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, const std::wstring& path);
};