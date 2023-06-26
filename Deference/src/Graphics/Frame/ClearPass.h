#pragma once

#include "TargetPass.h"

class ClearPass : public TargetPass
{
public:
	ClearPass(const std::string& name);
	virtual void Run(Graphics& g) override;

};