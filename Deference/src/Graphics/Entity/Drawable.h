#pragma once

#include "Step.h"
#include "Bindable/VertexBuffer.h"
#include "Bindable/IndexBuffer.h"

typedef std::pair<ComPtr<ID3D12Resource>, XMMATRIX> DXRInstance;

struct AccelerationStructureBuffers
{
	ComPtr<ID3D12Resource> pScratch;
	ComPtr<ID3D12Resource> pResult;
	ComPtr<ID3D12Resource> pInstanceDesc;
};

class Drawable
{
public:
	inline auto& GetInstance() const { return m_Instance; }
	inline auto& GetSteps() const { return m_Steps; }

protected:
	void CreateBottomLevelAS(Graphics& g, const std::vector<Shared<VertexBuffer>>& vVertexBuffers,
		const std::vector<Shared<IndexBuffer>>& indexBuffers);
	void AddStep(const Step& step);

private:
	DXRInstance m_Instance;
	std::vector<Step> m_Steps;
};

class DrawableCollection
{
public:
	inline auto& GetInstances() const { return m_Instances; }
	inline auto& GetSteps() const { return m_Steps; }
	void AddDrawable(const Drawable& d);
	
private:
	std::vector<DXRInstance> m_Instances;
	std::vector<Step> m_Steps;
};