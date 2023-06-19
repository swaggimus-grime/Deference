#pragma once

#include "Resource.h"

class Target : public Resource
{
public:
	Target(D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES state, ComPtr<ID3D12Resource> res = nullptr);
	virtual void Clear(Graphics& g) = 0;
	virtual void Bind(Graphics& g) = 0;
};