#pragma once

#include "Resource/Heap.h"
#include "Resource/RenderTarget.h"
#include "Resource/DepthStencil.h"
#include "Resource/ShaderResource.h"
#include "Entity/Drawable.h"
#include "Entity/Step.h"

class Graphics;
class Pass;
class Bindable;

class SceneData
{
public:
	void AddDrawableCollection(const DrawableCollection& d);
	void AddDrawable(const Drawable& d);
	ComPtr<ID3D12Resource> CreateTopLevelAS(Graphics& g) const;
	std::vector<Step> m_Steps;
	std::vector<DXRInstance> m_Instances;
	Shared<Camera> m_Camera;
};

class FrameGraph
{
public:
	FrameGraph(Graphics& g, const SceneData& scene);

	virtual void Run(Graphics& g);

	inline auto RT() const { return m_RT; }

protected:
	void AddPass(Graphics& g, Shared<Pass> pass);
	void AddBindable(Shared<Bindable> bindable);
	void Connect(const std::string& outPassTarget, Shared<Pass> inPass, const std::string& inTarget);
	void AddSteps(const std::vector<Step>& steps);
	Shared<Pass> GetPass(const std::string& name);

protected:
	Shared<RenderTarget> m_RT;
	Shared<DepthStencil> m_DS;

private:
	std::vector<Shared<Pass>> m_Passes;
	std::vector<Shared<Bindable>> m_Bindables;
	Heap<RenderTarget> m_RTHeap;
	Heap<DepthStencil> m_DSHeap;
};