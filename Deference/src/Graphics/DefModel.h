#pragma once

#include <Model.h>
#include <CommonStates.h>
#include <Effects.h>
#include <string>
#include "util.h"

class Graphics;

class DefModel
{
public:
	DefModel(Graphics& g, const std::wstring& filePath);

private:
	Unique<DirectX::Model> m_Model;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::EffectFactory> m_fxFactory;
	std::unique_ptr<DirectX::EffectTextureFactory> m_modelResources;
	DirectX::Model::EffectCollection m_modelNormal;
};