#pragma once

#include "RaytracePass.h"

class ShadowTracePass : public RaytracePass
{
public:
	ShadowTracePass(Graphics& g, const std::string& name, ComPtr<ID3D12Resource> topLevelAS, Shared<Camera> cam);

private:
	
};