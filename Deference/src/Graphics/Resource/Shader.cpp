#include "Shader.h"
#include "Graphics.h"
#include "DXC.h"

namespace Def
{
    Shader::Shader(const std::wstring& path, const std::wstring& type)
    {
        DXC::Compile(path, type, m_ByteCode);
        m_BCStruct = {};
        m_BCStruct.BytecodeLength = m_ByteCode->GetBufferSize();
        m_BCStruct.pShaderBytecode = m_ByteCode->GetBufferPointer();
    }

    VertexShader::VertexShader(const std::wstring& path)
        :Shader(path, L"vs")
    {
    }

    PixelShader::PixelShader(const std::wstring& path)
        :Shader(path, L"ps")
    {
    }
}