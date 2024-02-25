//#pragma once
//
//#include "Scene/Model.h"
//#include "RasterPass.h"
//
//namespace Def
//{
//	class Model;
//	struct SceneNode;
//
//	class DiffusePass : public RasterPass
//	{
//	public:
//		DiffusePass(Graphics& g, const std::string& name, FrameGraph* parent);
//		virtual void Run(Graphics& g) override;
//		virtual void OnSceneLoad(Graphics& g) override;
//		virtual void ShowGUI() override;
//
//	private:
//		void Rasterize(Graphics& g, XMMATRIX parentTransform, Model& model, Shared<SceneNode> node);
//
//	private:
//		Unique<ConstantBuffer> m_Material;
//		Unique<ConstantBuffer> m_Transform;
//		Unique<ConstantBuffer> m_Camera;
//		Unique<ConstantBuffer> m_Light;
//		Unique<SamplerHeap> m_SamplerHeap;
//	};
//}