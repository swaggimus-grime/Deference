#pragma once

#include "Resource.h"
#include <DirectXTex/DirectXTex.h>

class Texture2D : public Resource
{
public:
	Texture2D(Graphics& g, const std::wstring& path);
	virtual void CreateView(Graphics& g, HCPU hcpu) override;
};

class EnvironmentMap : public Resource
{
public:
	EnvironmentMap(Graphics& g, const std::wstring& path);
	virtual void CreateView(Graphics& g, HCPU hcpu) override;
	inline void SetHGPU(HGPU hgpu) { m_HGPU = hgpu; };
	inline auto GetHGPU() const { return m_HGPU; }
private:
	HGPU m_HGPU;
};