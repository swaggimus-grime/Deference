#pragma once

#include "RasterPass.h"

class LambertianPass : public RasterPass
{
public:
	LambertianPass(Graphics& g, const std::string& name);

};