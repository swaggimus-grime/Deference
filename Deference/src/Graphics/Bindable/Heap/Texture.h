#pragma once

#include "Resource.h"
#include <DirectXTex/DirectXTex.h>

class Texture2D : public Resource
{
public:
	Texture2D() = default;
	Texture2D(Graphics& g, const std::wstring& path);
	virtual void CreateView(Graphics& g, HCPU h) override;
};

class EnvironmentMap : public Resource
{
public:
	EnvironmentMap(Graphics& g, const std::wstring& path);
	virtual void CreateView(Graphics& g, HCPU h) override;
};