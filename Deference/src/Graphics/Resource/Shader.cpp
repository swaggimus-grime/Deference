#include "Shader.h"
#include "Graphics.h"
#include "DXC.h"

namespace Def
{
    Shader::Shader(const std::wstring& path, const std::wstring& type)
    {
        m_ByteCode = DXC::Compile(path);
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