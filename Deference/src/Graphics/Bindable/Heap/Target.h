#pragma once

#include "Resource.h"

class Target : public Resource, public Bindable
{
public:
	Target(D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res = nullptr, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON);
	virtual void Clear(Graphics& g) = 0;
	virtual void BindWithOther(Graphics& g, Target* other) = 0;
};