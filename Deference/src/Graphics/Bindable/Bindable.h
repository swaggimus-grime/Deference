#pragma once

class Graphics;

class Bindable
{
public:
	virtual void Bind(Graphics& g) = 0;
};