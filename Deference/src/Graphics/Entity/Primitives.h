#pragma once

#include "Drawable.h"

class Primitive : public Drawable
{
public:
	
};

class Plane : public Drawable
{
public:
	Plane(const XMFLOAT3& size = { 1.f, 1.f, 1.f });
};