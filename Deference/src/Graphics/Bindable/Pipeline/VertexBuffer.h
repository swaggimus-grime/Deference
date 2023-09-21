#pragma once

#include "InputLayout.h"
#include "Debug/Exception.h"
#include "util.h"
#include "Bindable/Bindable.h"

#define GET_ATTRIB(name, attr)   Map<attr>::type& name##(UINT idx) \
						{ \
							BR(idx >= 0 && idx < m_NumVertices); \
							return *reinterpret_cast<Map<attr>::type*>(m_Data.data() + idx * m_Stride + m_Offsets[attr]); \
						} \

class VertexStream
{
public:
	using enum VERTEX_ATTRIBUTES;

	VertexStream(const InputLayout& layout, UINT numVertices);

	GET_ATTRIB(Pos, POS)
	GET_ATTRIB(Color, COLOR)
	GET_ATTRIB(Tex, TEX)
	GET_ATTRIB(Norm, NORM)
	GET_ATTRIB(Tan, TAN)
	GET_ATTRIB(Bitan, BITAN)

	inline const void* Data() const { return m_Data.data(); }
	inline UINT Size() const { return m_Stride * m_NumVertices; }
	inline UINT Stride() const { return m_Stride; }

private:
	std::vector<CHAR> m_Data;
	UINT m_NumVertices;
	UINT m_Stride;
	std::unordered_map<VERTEX_ATTRIBUTES, UINT> m_Offsets;
};

class VertexBuffer : public Resource, public Bindable
{
public:
	VertexBuffer(Graphics& g, const VertexStream& stream);
	virtual void Bind(Graphics& g) override;
	virtual void CreateView(Graphics& g, HCPU h) override;
	inline UINT NumVertices() const { return m_View.SizeInBytes / Stride(); }
	inline UINT Stride() const { return m_View.StrideInBytes; }

private:
	D3D12_VERTEX_BUFFER_VIEW m_View;
};