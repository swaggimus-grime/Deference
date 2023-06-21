#include "DefModel.h"
#include <ResourceUploadBatch.h>
#include "Graphics.h"
#include <dxgi1_6.h>

DefModel::DefModel(Graphics& g, const std::wstring& filePath)
{
    m_states = std::make_unique<CommonStates>(&g.Device());
	m_Model = Model::CreateFromSDKMESH(&g.Device(), filePath.c_str());

    DirectX::ResourceUploadBatch resourceUpload(&g.Device());

    resourceUpload.Begin();

    m_modelResources = m_Model->LoadTextures(&g.Device(), resourceUpload);

    m_fxFactory = std::make_unique<EffectFactory>(m_modelResources->Heap(), m_states->Heap());

    auto uploadResourcesFinished = resourceUpload.End(&g.CQ());

    uploadResourcesFinished.wait();

    RenderTargetState rtState(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);

    EffectPipelineStateDescription pd(
        nullptr,
        CommonStates::Opaque,
        CommonStates::DepthDefault,
        CommonStates::CullClockwise,
        rtState);

    m_modelNormal = m_Model->CreateEffects(*m_fxFactory, pd, pd);
}
