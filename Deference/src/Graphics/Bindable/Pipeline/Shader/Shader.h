#pragma once

class Shader
{
public:
	Shader() = delete;
	inline const D3D12_SHADER_BYTECODE& ByteCode() const { return m_BCStruct; }
protected:
	Shader(const std::wstring& path, const std::wstring& type);

private:
	ComPtr<IDxcBlob> m_ByteCode;
	D3D12_SHADER_BYTECODE m_BCStruct;
};

class VertexShader : public Shader
{
public:
	VertexShader(const std::wstring& path);
};

class PixelShader : public Shader
{
public:
	PixelShader(const std::wstring& path);
};

