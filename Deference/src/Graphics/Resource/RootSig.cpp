#include "RootSig.h"
#include <d3dx12.h>

namespace Def
{
    RootSig::RootSig(Graphics& g, SIZE_T numParams, const CD3DX12_ROOT_PARAMETER1* params, bool isLocal)
    {
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc;
        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        if (isLocal)
            flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        desc.Init_1_1(numParams, params, 0, nullptr, flags);

        ComPtr<ID3DBlob> blob;
        ComPtr<ID3DBlob> errorBlob;
        if (FAILED(D3DX12SerializeVersionedRootSignature(&desc,
            Graphics::ROOT_SIG_VERSION, &blob, &errorBlob)))
            throw std::runtime_error(static_cast<CHAR*>(errorBlob->GetBufferPointer()));

        HR g.Device().CreateRootSignature(0, blob->GetBufferPointer(),
            blob->GetBufferSize(), IID_PPV_ARGS(&m_Sig));
    }

    void RootSig::Bind(Graphics& g)
    {
        g.CL().SetGraphicsRootSignature(m_Sig.Get());
    }
}