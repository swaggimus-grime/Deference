#include "Shader.h"
#include <d3dcompiler.h>

Shader::Shader(const std::wstring& path, const std::string& type)
{
    ComPtr<ID3DBlob> err;
    if(FAILED(D3DCompileFromFile(path.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        (type + "_5_1").c_str(),
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &m_ByteCode,
        &err)))
        throw std::runtime_error(static_cast<char*>(err->GetBufferPointer()));
        
    m_BCStruct = {};
    m_BCStruct.BytecodeLength = m_ByteCode->GetBufferSize();
    m_BCStruct.pShaderBytecode = m_ByteCode->GetBufferPointer();
}

VertexShader::VertexShader(const std::wstring& path)
    :Shader(path, "vs")
{
}

PixelShader::PixelShader(const std::wstring& path)
    :Shader(path, "ps")
{
}