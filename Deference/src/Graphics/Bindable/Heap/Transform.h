#pragma once

#include "ConstantBuffer.h"
#include "util.h"

struct TransformParams
{
	XMMATRIX m_MVP;
	XMFLOAT3X3 m_NormMat;
	XMMATRIX m_Model;
};

class Transform : public ConstantBuffer<TransformParams>
{
public:
	Transform(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	virtual void Update(Graphics& g) override;

private:
	XMFLOAT3 m_Pos;
	XMFLOAT3 m_Rot;
	XMFLOAT3 m_Scale;

	XMMATRIX m_PosMat;
	XMMATRIX m_RotMat;
	XMMATRIX m_ScaleMat;
};