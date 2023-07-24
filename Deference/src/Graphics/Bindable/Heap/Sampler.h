#pragma once

#include "Resource.h"

class Sampler
{
public:
	Sampler(Graphics& g, HCPU handle);

private:
	HCPU m_Handle;
};