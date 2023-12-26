#pragma once

#include "ScreenPass.h"
#include "Bindable/Heap/RenderTarget.h"

class CopyPass : public ScreenPass
{
public:
	CopyPass(Graphics& g, const std::string& name, FrameGraph* parent);
	virtual void Run(Graphics& g) override;

private:

};