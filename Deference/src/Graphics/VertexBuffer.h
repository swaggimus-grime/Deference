#pragma once

#include <unordered_map>
#include "Debug/Exception.h"
#include "util.h"

enum class VERTEX_ATTRIBUTES
{
	POS = 0,
	TEX = 0x1,
	NORM = 0x2,
	TAN = 0x4,
	BITAN = 0x8,
	COLOR = 0x10
};

template<VERTEX_ATTRIBUTES> struct Map;

template<> struct Map<VERTEX_ATTRIBUTES::POS> {
	using type = XMFLOAT3;
	static constexpr auto name = "POSITION";
	static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
};

template<> struct Map<VERTEX_ATTRIBUTES::TEX> {
	using type = XMFLOAT2;
	static constexpr auto name = "TEXCOORD";
	static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32_FLOAT;
};

template<> struct Map<VERTEX_ATTRIBUTES::NORM> {
	using type = XMFLOAT3;
	static constexpr auto name = "NORMAL";
	static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
};

template<> struct Map<VERTEX_ATTRIBUTES::TAN> {
	using type = XMFLOAT3;
	static constexpr auto name = "TANGENT";
	static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
};

template<> struct Map<VERTEX_ATTRIBUTES::BITAN> {
	using type = XMFLOAT3;
	static constexpr auto name = "BITANGENT";
	static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
};

template<> struct Map<VERTEX_ATTRIBUTES::COLOR> {
	using type = XMFLOAT4;
	static constexpr auto name = "COLOR";
	static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
};

inline VERTEX_ATTRIBUTES operator&(VERTEX_ATTRIBUTES a, VERTEX_ATTRIBUTES b)
{
	return static_cast<VERTEX_ATTRIBUTES>(static_cast<int>(a) & static_cast<int>(b));
}

inline VERTEX_ATTRIBUTES operator|(VERTEX_ATTRIBUTES a, VERTEX_ATTRIBUTES b)
{
	return static_cast<VERTEX_ATTRIBUTES>(static_cast<int>(a) | static_cast<int>(b));
}

inline void operator|=(VERTEX_ATTRIBUTES& a, VERTEX_ATTRIBUTES b)
{
	a = static_cast<VERTEX_ATTRIBUTES>(static_cast<int>(a) | static_cast<int>(b));
}

#define GET_ATTRIB(name, attr)   Map<attr>::type& name##(UINT idx) \
						{ \
							BR(idx >= 0 && idx < m_NumVertices); \
							return *reinterpret_cast<Map<attr>::type*>(m_Data.data() + idx * m_Stride + m_Offsets[attr]); \
						} \

class InputLayout
{
public:
	friend class VertexStream;

	InputLayout(const VERTEX_ATTRIBUTES& attributes);
	inline D3D12_INPUT_LAYOUT_DESC Layout() const { return m_Layout; }

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_Elements;
	D3D12_INPUT_LAYOUT_DESC m_Layout;
	const VERTEX_ATTRIBUTES m_Attributes;
	UINT m_Stride;
	std::unordered_map<VERTEX_ATTRIBUTES, UINT> m_Offsets;
};

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

class VertexBuffer
{
public:
	VertexBuffer(Graphics& g, const VertexStream& stream);
	void Bind(Graphics& g) const;
	inline auto* Res() const { return m_Buffer.Get(); }
	inline UINT NumVertices() const { return m_View.SizeInBytes / Stride(); }
	inline UINT Stride() const { return m_View.StrideInBytes; }

private:
	ComPtr<ID3D12Resource> m_Buffer;
	D3D12_VERTEX_BUFFER_VIEW m_View;
};