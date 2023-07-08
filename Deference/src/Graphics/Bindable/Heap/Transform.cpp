#include "Transform.h"
#include "Graphics.h"

Transform::Transform(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle)
	:ConstantBuffer(g, handle)
{
}

void Transform::Update(Graphics& g)
{
	auto cam = g.GetCamera();
	TransformParams t;
	t.m_MVP = XMMatrixTranspose(m_Model * cam->View() * cam->Proj());
	XMStoreFloat3x3(&t.m_NormMat, XMMatrixInverse(nullptr, m_Model));

	ConstantBuffer::Update(&t);
}