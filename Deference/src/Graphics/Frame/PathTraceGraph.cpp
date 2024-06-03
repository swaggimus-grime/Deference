#include "PathTraceGraph.h"
#include "DiffuseGIPass.h"
#include "Scene/Model.h"
#include "CopyPass.h"
#include "Resource/Buffer.h"

namespace Def
{
	PathTraceGraph::PathTraceGraph(Graphics& g)
	{
		AddPass<RaytracedGeometryPass>(g, "Geometry");
		{
			auto pass = AddPass<DiffuseGIPass>(g, "Diffuse and GI");
			pass->Link("Geometry", "Position");
			pass->Link("Geometry", "Normal");
			pass->Link("Geometry", "Albedo");
			pass->Link("Geometry", "Specular");
			pass->Link("Geometry", "Emissive");
		}
		{
			auto pass = AddPass<AccumPass>(g, "GI accum");
			pass->Link("Diffuse and GI", "Target");
		}
		/*{
			auto pass = AddPass<ToneMapPass>(g, "Tonemap");
			pass->Link("GI accum", "Target", "HDR");
		}*/
	}

	
}