#pragma once

#include "ScreenPass.h"

class HybridOutPass : public ScreenPass
{
public:
	HybridOutPass(Graphics& g, const std::string& name, FrameGraph* parent);
	virtual void Run(Graphics& g) override;
};