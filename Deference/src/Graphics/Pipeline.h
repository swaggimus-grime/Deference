#pragma once

class RootSig;
class VertexShader;
class PixelShader;

class Pipeline
{
public:
	Pipeline(Graphics& g, const RootSig& rootSig, const VertexShader& vs, const PixelShader& ps, const D3D12_INPUT_LAYOUT_DESC& inputLayout);
	inline ID3D12PipelineState* State() const { return m_State.Get(); }

private:
	ComPtr<ID3D12PipelineState> m_State;
};