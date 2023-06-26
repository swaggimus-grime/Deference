#pragma once

#include "Resource.h"

class Texture2D : public Resource
{
public:
	Texture2D(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, const std::wstring& path);
};