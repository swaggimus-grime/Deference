#pragma once

#include "Bindable/Bindable.h"

class RootSig : public Bindable
{
public:
	RootSig(Graphics& g, SIZE_T numParams, const CD3DX12_ROOT_PARAMETER1* params, bool isLocal = false);
	inline auto* Sig() const { return m_Sig.Get(); }
	virtual void Bind(Graphics& g) override;

private:
	ComPtr<ID3D12RootSignature> m_Sig;
};