#pragma once

#include "ConstantBuffer.h"
#include "util.h"

struct TransformParams
{
	XMMATRIX m_MVP;
	XMFLOAT3X3 m_NormMat;
};

class Transform : public ConstantBuffer<TransformParams>
{
public:
	Transform(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void Update(Graphics& g);

private:
	XMMATRIX m_Model = XMMatrixIdentity();
};