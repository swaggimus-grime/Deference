#pragma once

#include "InputLayout.h"
#include "Debug/Exception.h"
#include "util.h"
#include "Bindable.h"

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
	GET_ATTRIB(Tex, TEX)
	GET_ATTRIB(Norm, NORM)
	GET_ATTRIB(Tan, TAN)
	GET_ATTRIB(Bitan, BITAN)
	GET_ATTRIB(Color, COLOR)

	inline const void* Data() const { return m_Data.data(); }
	inline UINT Size() const { return m_Stride * m_NumVertices; }
	inline UINT Stride() const { return m_Stride; }

private:
	std::vector<CHAR> m_Data;
	UINT m_NumVertices;
	UINT m_Stride;
	std::unordered_map<VERTEX_ATTRIBUTES, UINT> m_Offsets;
};

class VertexBuffer : public Bindable
{
public:
	VertexBuffer(Graphics& g, const VertexStream& stream);
	virtual void Bind(Graphics& g) override;
	inline auto* Res() const { return m_Buffer.Get(); }
	inline UINT NumVertices() const { return m_View.SizeInBytes / Stride(); }
	inline UINT Stride() const { return m_View.StrideInBytes; }

private:
	ComPtr<ID3D12Resource> m_Buffer;
	D3D12_VERTEX_BUFFER_VIEW m_View;
};