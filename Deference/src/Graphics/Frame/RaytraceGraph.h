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
		Shared<RawBuffer> Indices;
		Shared<StructuredBuffer> UV_0;
		Shared<StructuredBuffer> UV_1;
		Shared<StructuredBuffer> UV_2;
		Shared<StructuredBuffer> Normals;
		Shared<StructuredBuffer> Tangents;
		UINT MaterialID;

		Geometry(
			const Shared<StructuredBuffer>& v,
			const Shared<RawBuffer>& i,
			const Shared<StructuredBuffer>& u0,
			const Shared<StructuredBuffer>& u1,
			const Shared<StructuredBuffer>& u2,
			const Shared<StructuredBuffer>& n, 
			const Shared<StructuredBuffer>& t, 
			UINT matID)
			:Vertices(v), Indices(i), UV_0(u0), UV_1(u1), UV_2(u2), Normals(n), Tangents(t), MaterialID(matID)
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