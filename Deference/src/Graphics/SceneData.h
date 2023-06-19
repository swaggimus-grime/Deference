#pragma once

#include <wrl.h>
#include <vector>

class VertexBuffer;
class IndexBuffer;
class InputLayout;

struct SceneData
{
	std::vector<Shared<VertexBuffer>> m_VBs;
	std::vector<Shared<IndexBuffer>> m_IBs;
	std::vector<Shared<InputLayout>> m_Layouts;

	inline void AddObject(Shared<VertexBuffer> vb, Shared<IndexBuffer> ib, Shared<InputLayout> layout)
	{
		m_VBs.push_back(std::move(vb));
		m_IBs.push_back(std::move(ib));
		m_Layouts.push_back(std::move(layout));
	}
};