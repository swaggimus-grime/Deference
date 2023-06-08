#pragma once

class Texture2D
{
public:
	Texture2D(Graphics& g, const std::wstring_view& path);
	void Bind(Graphics& g) const;

private:
	ComPtr<ID3D12Resource> m_Res;
	ComPtr<ID3D12DescriptorHeap> m_DescHeap;
};