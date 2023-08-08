#include "PathTraceGraph.h"

PathTraceGraph::PathTraceGraph(Graphics& g)
{
	AddGlobalResource("Env", MakeShared<EnvironmentMap>(g, L"textures\\MonValley_G_DirtRoad_3k.hdr"));

	AddPass<RaytracedGeometryPass>(g);
	AddPass<DiffusePass>(g);
	AddPass<AOPass>(g);
	AddPass<AccumPass>(g);
	AddPass<HybridOutPass>(g);
}
