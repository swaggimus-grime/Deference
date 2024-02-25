#include "DXC.h"
#include <d3dcompiler.h>
#include <dxcapi.h>
#include "Debug/Exception.h"
#include <fstream>

namespace Def
{
    namespace DXC
    {
        void Compile(const std::wstring& path, const std::wstring& type, ComPtr<IDxcBlob>& byteCode)
        {
            ComPtr<IDxcUtils> pUtils;
            ComPtr<IDxcCompiler3> pCompiler;
            DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
            DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
            ComPtr<IDxcIncludeHandler> pIncludeHandler;
            HR pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

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
            HR pUtils->LoadFile(path.c_str(), nullptr, &pSource);
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

            pResults->GetResult(&byteCode);
        }

        struct DXC_LibCompiler
        {
            IDxcCompiler* m_Compiler;
            IDxcLibrary* m_Lib;
            IDxcIncludeHandler* m_Includer;

            DXC_LibCompiler()
            {
                HR DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), reinterpret_cast<void**>(&m_Compiler));
                HR DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), reinterpret_cast<void**>(&m_Lib));
                HR m_Lib->CreateIncludeHandler(&m_Includer);
            }
        };

        static DXC_LibCompiler s_Compiler;

        void Compile(const std::wstring& path, ComPtr<IDxcBlob>& byteCode)
        {
            std::ifstream shaderFile(path);
            BR shaderFile.good();
            std::stringstream ss;
            ss << shaderFile.rdbuf();
            auto sstr = ss.str();

            IDxcBlobEncoding* enc;
            HR s_Compiler.m_Lib->CreateBlobWithEncodingFromPinned(
                LPBYTE(sstr.c_str()), static_cast<uint32_t>(sstr.size()), 0, &enc);

            // Compile
            IDxcOperationResult* result;
            LPCWSTR args = L"-enable-16bit-types";
            HR s_Compiler.m_Compiler->Compile(enc, path.c_str(), L"", L"lib_6_3", &args, 1, nullptr, 0, s_Compiler.m_Includer, &result);
            // Verify the result
            HRESULT resultCode;
            HR result->GetStatus(&resultCode);
            if (FAILED(resultCode))
            {
                IDxcBlobEncoding* pError;
                HR result->GetErrorBuffer(&pError);

                // Convert error blob to a string
                std::vector<char> infoLog(pError->GetBufferSize() + 1);
                memcpy(infoLog.data(), pError->GetBufferPointer(), pError->GetBufferSize());
                infoLog[pError->GetBufferSize()] = 0;

                std::string errorMsg = "Shader Compiler Error:\n";
                errorMsg.append(infoLog.data());

                MessageBoxA(nullptr, errorMsg.c_str(), "Error!", MB_OK);
                throw std::logic_error("Failed compile shader");
            }

            HR result->GetResult(&byteCode);
        }
    }
}