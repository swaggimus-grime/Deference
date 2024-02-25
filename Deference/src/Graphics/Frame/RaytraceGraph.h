#pragma once

#include "FrameGraph.h"
#include "Scene/Model.h"
#include "Resource/Buffer.h"
#include "Resource/AccelStruct.h"

namespace Def
{
	struct Geometry
	{
		Shared<StructuredBuffer> Vertices;
		Shared<ByteBuffer> Indices;
		Shared<StructuredBuffer> UVs;
		Shared<StructuredBuffer> Normals;
		Shared<StructuredBuffer> Tangents;
		UINT MaterialID;

		Geometry(
			const Shared<StructuredBuffer>& v, 
			const Shared<ByteBuffer>& i,
			const Shared<StructuredBuffer>& u,
			const Shared<StructuredBuffer>& n, 
			const Shared<StructuredBuffer>& t, 
			UINT matID)
			:Vertices(v), Indices(i), UVs(u), Normals(n), Tangents(t), MaterialID(matID)
		{}
	};

	class RaytraceGraph : public FrameGraph
	{
	public:
		struct Globals
		{
			Shared<StructuredBuffer> MatBuff;
			Shared<EnvironmentMap> Env;
			Shared<TLAS> TLAS;
			std::vector<Geometry> Geometries;
		};

	protected:
		virtual void PrepLoadScene(Graphics& g) override;

	private:
		Globals m_Globals;
	};
}