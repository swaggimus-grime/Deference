#include "GeometryGraph.h"
#include "Entity/Drawable.h"

GeometryGraph::GeometryGraph()
{

}

void GeometryGraph::AddGeometry(Shared<Drawable> d)
{
	m_Drawables.push_back(std::move(d));
}

void GeometryGraph::AddGeometry(DrawableCollection& d)
{
	for (auto& drawable : d.Drawables())
		AddGeometry(std::move(drawable));
}

void GeometryGraph::AddPass(Graphics& g, Shared<Pass> pass)
{
	ConnectTargets(pass);
	pass->OnAdd(g, this);
	for (auto& out : pass->GetOutTargets())
		m_Targets.push_back(out);
	m_Passes.push_back(std::move(pass));
}

Shared<RenderTarget> GeometryGraph::Run(Graphics& g)
{
	for (auto& p : m_Passes)
		p->Run(g, this);

	return GetTarget("Hybrid");
}

void GeometryGraph::ConnectTargets(Shared<Pass> pass)
{
	if (m_Passes.empty())
		return;

	for (auto& in : pass->GetInTargets())
		in.second = GetTarget(in.first);
}

Shared<RenderTarget> GeometryGraph::GetTarget(const std::string& name)
{
	auto it = std::find_if(m_Targets.begin(), m_Targets.end(), [&](const auto& p) { return name == p.first; });
	return it->second;
}
