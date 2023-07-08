#pragma once

#include <unordered_map>
#include <d3d12.h>
#include "Frame/GeometryPass.h"

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

enum class INPUT_LAYOUT_CONFIG
{
	GEOMETRY_PIPELINE
};

class InputLayout
{
public:
	friend class VertexStream;

	InputLayout(const VERTEX_ATTRIBUTES& attributes);
	InputLayout(INPUT_LAYOUT_CONFIG config = INPUT_LAYOUT_CONFIG::GEOMETRY_PIPELINE);
	inline D3D12_INPUT_LAYOUT_DESC Layout() const { return m_Layout; }
	inline UINT NumElements() const { return m_Elements.size(); }

private:
	VERTEX_ATTRIBUTES MapConfigToAttribs(INPUT_LAYOUT_CONFIG config);

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_Elements;
	D3D12_INPUT_LAYOUT_DESC m_Layout;
	const VERTEX_ATTRIBUTES m_Attributes;
	UINT m_Stride;
	std::unordered_map<VERTEX_ATTRIBUTES, UINT> m_Offsets;
};