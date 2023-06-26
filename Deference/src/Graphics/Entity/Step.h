#pragma once

#include "Graphics.h"
#include "Frame/Pass.h"
#include "util.h"
#include <unordered_map>

class VertexBuffer;
class IndexBuffer;

class Step
{
public:
	Step(const std::string& passName, UINT idxCount);
	inline std::string PassName() const { return m_PassName; }
	inline void AddBindable(Shared<Bindable> bindable)
	{
		m_Bindables.push_back(std::move(bindable));
	}

	void Run(Graphics& g);

private:
	std::string m_PassName;
	std::vector<Shared<Bindable>> m_Bindables;
	UINT m_IdxCount;
};