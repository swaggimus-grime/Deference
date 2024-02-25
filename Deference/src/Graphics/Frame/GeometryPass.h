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
//	class GeometryPass : public RasterPass
//	{
//	public:
//		GeometryPass(Graphics& g, const std::string& name, FrameGraph* parent);
//		virtual void Run(Graphics& g) override;
//		virtual void OnSceneLoad(Graphics& g) override;
//
//	private:
//		void Rasterize(Graphics& g, XMMATRIX parentTransform, Model& model, Shared<SceneNode> node);
//
//	private:
//		Unique<ConstantBuffer> m_Material;
//		Shared<ConstantBuffer> m_Transform;
//		Shared<ConstantBuffer> m_Camera;
//		Unique<SamplerHeap> m_SamplerHeap;
//	};
//}