#pragma once

#include "util.h"
#include <unordered_map>
#include <string>
#include <optional>

class Graphics;
class Target;
class Step;

class Pass
{
public:
	Pass(const std::string& name);

	inline std::string Name() const { return m_Name; }
	virtual void Run(Graphics& g) = 0;
	Shared<Target>& GetTarget(const std::string& name);
	virtual void OnAdd(Graphics& g) = 0;

	void AddStep(const Step& step);

protected:
	void AddTarget(const std::string& name);
	std::vector<Step> m_Steps;

private:
	std::unordered_map<std::string, Shared<Target>> m_Targets;
	std::string m_Name;
};