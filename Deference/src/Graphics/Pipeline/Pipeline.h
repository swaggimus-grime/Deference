#pragma once

class RootSig;
class VertexShader;
class PixelShader;
class InputLayout;

class Pipeline : public Bindable
{
public:
	Pipeline(Graphics& g, const RootSig& rootSig, const VertexShader& vs, const PixelShader& ps, const InputLayout& inputLayout);
	virtual void Bind(Graphics& g) override;

private:
	ComPtr<ID3D12PipelineState> m_State;
};