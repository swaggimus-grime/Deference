#pragma once

#include "ConstantBuffer.h"
#include "util.h"

struct AOConstantsParams
{
    float radius = 100.f;
    unsigned int frameCount = 0;
    float minT = 1.0e-4f;
};

class AOConstants : public ConstantBuffer<AOConstantsParams>
{
public:
	AOConstants(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle);
    void Update(UINT frameCount);
};