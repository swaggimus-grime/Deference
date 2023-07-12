#include "Shader.h"
#include <d3dcompiler.h>
#include "Graphics.h"
#include <dxcapi.h>

Shader::Shader(const std::wstring& path, const std::wstring& type)
{
    ComPtr<IDxcUtils> pUtils;
    ComPtr<IDxcCompiler3> pCompiler;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
    ComPtr<IDxcIncludeHandler> pIncludeHandler;
    pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

    std::wstring target = type + L"_6_3";

    LPCWSTR pszArgs[] =
    {
        path.c_str(),            // Optional shader source file name for error reporting
        // and for PIX shader source view.  
        L"-E", L"main",              // Entry point.
        L"-T", target.c_str(),            // Target.
        L"-Zs"                      // Enable debug information (slim format)
    };


    //
    // Open source file.  
    //
    ComPtr<IDxcBlobEncoding> pSource = nullptr;
    pUtils->LoadFile(path.c_str(), nullptr, &pSource);
    DxcBuffer Source;
    Source.Ptr = pSource->GetBufferPointer();
    Source.Size = pSource->GetBufferSize();
    Source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

    // Compile it with specified arguments.
    //
    ComPtr<IDxcResult> pResults;
    pCompiler->Compile(
        &Source,                // Source buffer.
        pszArgs,                // Array of pointers to arguments.
        _countof(pszArgs),      // Number of arguments.
        pIncludeHandler.Get(),        // User-provided interface to handle #include directives (optional).
        IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
    );

    //
    // Print errors if present.
    //
    ComPtr<IDxcBlobUtf8> pErrors = nullptr;
    pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
    // Note that d3dcompiler would return null if no errors or warnings are present.
    // IDxcCompiler3::Compile will always return an error buffer, but its length
    // will be zero if there are no warnings or errors.
    if (pErrors != nullptr && pErrors->GetStringLength() != 0)
        throw std::runtime_error(pErrors->GetStringPointer());

    pResults->GetResult(&m_ByteCode);
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