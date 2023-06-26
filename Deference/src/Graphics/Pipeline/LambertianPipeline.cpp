#include "LambertianPipeline.h"
#include "Shader/Shader.h"
#include "Bindable/InputLayout.h"

LambertianPipeline::LambertianPipeline(Graphics& g, const RootSig& sig)
	:Pipeline(g, sig, VertexShader(L"shaders\\PosColorVS.hlsl"), PixelShader(L"shaders\\PosColorPS.hlsl"), 
		InputLayout::s_PosTexColor)
{
}
