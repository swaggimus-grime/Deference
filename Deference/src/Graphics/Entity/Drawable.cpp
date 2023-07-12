#include "Drawable.h"
#include "Graphics.h"
#include "Bindable/Pipeline/VertexBuffer.h"
#include "Bindable/Pipeline/IndexBuffer.h"
#include "Bindable/Heap/Transform.h"

void Drawable::Update(Graphics& g)
{
	m_Transform->Update(g);
}

void Drawable::Rasterize(Graphics& g)
{
	Update(g);

	for (auto& b : m_Bindables)
		b->Bind(g);
	m_VB->Bind(g);
	m_IB->Bind(g);

	g.CL().DrawIndexedInstanced(m_IB->NumIndices(), 1, 0, 0, 0);
}