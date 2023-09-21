#pragma once

#include "Bindable/Heap/DescriptorHeap.h"
#include "Swapchain.h"

class Graphics;
class FrameGraph;
class RenderTargetHeap;
class Viewport;

struct PassTargetName : private std::pair<std::string, std::string>
{
	std::string Pass;
	std::string Target;

	PassTargetName(std::string pass, std::string target)
		:std::pair<std::string, std::string>(pass, target), 
		Pass(std::move(this->first)), Target(std::move(this->second))
	{
	}
};

class Pass
{
public:
	Pass(const std::string& name, FrameGraph* parent);

	inline auto GetName() const { return m_Name; }
	inline const auto& GetInTargets() const { return m_InTargets; }
	inline const auto& GetOutTargets() const { return m_OutTargets; }
	inline const auto& GetInTarget(const std::string& target) const { 
		auto it = std::find_if(m_InTargets.begin(), m_InTargets.end(),
			[&](const auto& p) {
				return p.first == target;
			});
		if (it != m_InTargets.end())
			return it->second;
		else
			throw DefException("Cannot find in target with name " + target);
	}
	inline const auto& GetOutTarget(const std::string& target) const {
		auto it = std::find_if(m_OutTargets.begin(), m_OutTargets.end(),
			[&](const auto& p) {
				const auto& [name, format, t] = p;
				return name == target;
			});
		if (it != m_OutTargets.end())
			return std::get<2>(*it);
		else
			throw DefException("Cannot find out target with name " + target);
	}

	void Link(const std::string& otherPass, const std::string& otherTarget, std::optional<std::string> targetName = {});
	virtual void Finish(Graphics& g);
	virtual void Run(Graphics& g) = 0;
	virtual void ShowGUI() {}
	virtual void OnResize(Graphics& g, UINT w, UINT h);
	
protected:
	void AddResource(Shared<Resource> r);
	void AddTargetResource(Shared<Target> r);
	void QueryGlobalResource(const std::string& name);
	void QueryGlobalVectorResource(const std::string& name);


	inline const auto& GetGlobalResource(const std::string& name)
	{
		return m_GlobalResources[name];
	}

	inline const auto& GetGlobalVectorResource(const std::string& name)
	{
		return m_GlobalVectorResources[name];
	}

	inline const auto& GetResource(const Shared<Resource>& r)
	{
		auto it = std::find_if(m_Resources.begin(), m_Resources.end(), 
		[&](const auto& p) {
			return p.first == r;
		});
		return it->second;
	}

	inline void AddOutTarget(std::string&& target, DXGI_FORMAT fmt = Swapchain::s_Format) {
		m_OutTargets.emplace_back(std::move(target), fmt, nullptr);
	}

	inline void AddInTarget(std::string&& target) {
		m_InTargets.emplace_back(std::move(target), nullptr);
	}

	inline void AddBindable(Shared<Bindable> b) { m_Bindables.push_back(std::move(b)); }
	void BindBindables(Graphics& g);
	
protected:
	Unique<GPUShaderHeap> m_GPUHeap;
	Unique<RenderTargetHeap> m_TargetHeap;
	FrameGraph* m_Parent;

private:
	Shared<Viewport> m_Viewport;
	std::vector<Shared<Bindable>> m_Bindables;

	std::vector<std::pair<Shared<Resource>, HGPU>> m_Resources;
	std::vector<Shared<Target>> m_TargetResources;
	std::unordered_map<std::string, HGPU> m_GlobalResources;
	std::unordered_map<std::string, std::vector<std::vector<HGPU>>> m_GlobalVectorResources;

	std::string m_Name;
	std::vector<std::pair<std::string, Shared<RenderTarget>>> m_InTargets;
	std::vector<std::tuple<std::string, DXGI_FORMAT, Shared<RenderTarget>>> m_OutTargets;
};