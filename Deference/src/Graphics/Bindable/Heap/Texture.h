#pragma once

#include "Resource.h"
#include <DirectXTex/DirectXTex.h>

class Texture2D : public Resource
{
public:
	Texture2D(Graphics& g, const std::wstring& path);
	virtual void CreateView(Graphics& g, HCPU hcpu) override;
};

//class EnvironmentMap : public Texture
//{
//public:
//	EnvironmentMap(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, const std::wstring& path);
//};