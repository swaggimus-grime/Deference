#include "DXC.h"
#include <d3dcompiler.h>
#include <dxcapi.h>
#include "Debug/Exception.h"
#include <fstream>

namespace Def
{
    namespace DXC
    {
        struct DXC_Compiler
        {
            IDxcCompiler* Compiler;
            IDxcLibrary* Lib;
            IDxcIncludeHandler* Includer;

            DXC_Compiler()
            {
                HR DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), reinterpret_cast<void**>(&Compiler));
                HR DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), reinterpret_cast<void**>(&Lib));
                HR Lib->CreateIncludeHandler(&Includer);
            }
        };

        static DXC_Compiler s_DXC;

        ComPtr<IDxcBlob> Compile(std::wstring_view path)
        {
            IDxcBlobEncoding* enc;
            std::string shaderString;
            {
                std::ifstream shaderFile(path.data());
                BR shaderFile.good();
                std::stringstream ss;
                ss << shaderFile.rdbuf();
                shaderString = ss.str();
                HR s_DXC.Lib->CreateBlobWithEncodingFromPinned(
                    LPBYTE(shaderString.c_str()), static_cast<uint32_t>(shaderString.size()), 0, &enc);
            }

            std::wstring type;
            {
                const size_t typeStart = path.find_first_of(L'.') + 1;
                const size_t typeEnd = path.find_last_of(L'.');
                type = path.substr(typeStart, typeEnd - typeStart);
            }
            const auto target = (type != L"rt" ? type : L"lib") + L"_6_3";
            IDxcOperationResult* result;
            LPCWSTR args = L"-enable-16bit-types";
            HR s_DXC.Compiler->Compile(enc, path.data(), type != L"rt" ? L"main" : L"", target.c_str(), &args, 1, nullptr, 0, s_DXC.Includer, &result);

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
                throw DefException("Failed compile shader");
            }

            ComPtr<IDxcBlob> byteCode;
            HR result->GetResult(&byteCode);
            return std::move(byteCode);
        }
    }
}