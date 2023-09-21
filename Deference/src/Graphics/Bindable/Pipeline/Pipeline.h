#pragma once

class RootSig;
class VertexShader;
class PixelShader;
class InputLayout;

class Pipeline : public Bindable
{
public:
	virtual void Bind(Graphics& g) override;

protected:
	void Create(Graphics& g, Shared<RootSig> sig, const VertexShader& vs, const PixelShader& ps, 
		const InputLayout& layout, std::vector<DXGI_FORMAT> rtFormats);

private:
	ComPtr<ID3D12PipelineState> m_State;
	Shared<RootSig> m_Sig;
};