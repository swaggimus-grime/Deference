#pragma once

#include "RasterPass.h"

class ScreenPass : public RasterPass
{
public:
	ScreenPass(Graphics& g, const std::string& name, FrameGraph* parent);

protected:
	void Rasterize(Graphics& g);

private:
	UINT m_NumIndices;
};