#pragma once

#include "RasterPass.h"

class ScreenPass : public RasterPass
{
public:
	ScreenPass(Graphics& g);

protected:
	void Rasterize(Graphics& g);

private:
	UINT m_NumIndices;
};