#pragma once

class Context
{
public:
	virtual void Render(Graphics& g, Shared<RenderTarget> bb) = 0;
};