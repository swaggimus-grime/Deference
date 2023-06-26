#pragma once

#include "Pipeline.h"

class LambertianPipeline : public Pipeline
{
public:
	LambertianPipeline(Graphics& g, const RootSig& sig);
};