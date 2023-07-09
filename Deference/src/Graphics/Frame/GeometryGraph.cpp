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
	ConnectWithPreviousTarget(pass);
	pass->OnAdd(g, this);
	m_Passes.push_back(std::move(pass));
}

Shared<RenderTarget> GeometryGraph::Run(Graphics& g)
{
	for (auto& p : m_Passes)
		p->Run(g, this);

	return m_Passes[m_Passes.size() - 1]->GetOutTarget("Diffuse");
}

void GeometryGraph::ConnectWithPreviousTarget(Shared<Pass> pass)
{
	if (m_Passes.empty())
		return;

	auto& prevPassOuts = m_Passes[m_Passes.size() - 1]->GetOutTargets();
	auto& currentPassIns = pass->GetInTargets();
	for (auto& out : prevPassOuts)
		currentPassIns.push_back(out);
}
