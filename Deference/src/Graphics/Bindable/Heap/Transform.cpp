#include "Transform.h"
#include "Graphics.h"

Transform::Transform(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle)
	:ConstantBuffer(g, handle)
{
	m_Pos = m_Rot = m_Scale = XMFLOAT3();
	m_PosMat = m_RotMat = m_ScaleMat = XMMatrixIdentity();
}

void Transform::Update(Graphics& g)
{
	auto cam = g.GetCamera();
	auto model = m_PosMat * m_RotMat * m_ScaleMat;
	TransformParams t;
	t.m_MVP = XMMatrixTranspose(model * cam->View() * cam->Proj());
	XMStoreFloat3x3(&t.m_NormMat, XMMatrixInverse(nullptr, model));
	t.m_Model = XMMatrixTranspose(model);

	UpdateStruct(&t);
}