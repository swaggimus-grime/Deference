#pragma once

namespace Def
{
	class RootSig;
	class VertexShader;
	class PixelShader;
	class InputLayout;

	class Pipeline : public Bindable
	{
	public:
		Pipeline(Graphics& g, Shared<RootSig> sig, const VertexShader& vs, const PixelShader& ps,
			const D3D12_INPUT_LAYOUT_DESC& layout, std::vector<DXGI_FORMAT> rtFormats);
		virtual void Bind(Graphics& g) override;

	private:
		ComPtr<ID3D12PipelineState> m_State;
		Shared<RootSig> m_Sig;
	};
}