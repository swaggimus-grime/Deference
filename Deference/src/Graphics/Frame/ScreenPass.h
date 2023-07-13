#pragma once

#include "Pass.h"

class ScreenPass : public Pass
{
public:
	ScreenPass(Graphics& g);

protected:
	void Rasterize(Graphics& g);

private:
	UINT m_NumIndices;
};