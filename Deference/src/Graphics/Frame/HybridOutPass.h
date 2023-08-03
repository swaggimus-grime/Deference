#pragma once

#include "ScreenPass.h"

class HybridOutPass : public ScreenPass
{
public:
	HybridOutPass(Graphics& g);
	virtual void Run(Graphics& g) override;
};