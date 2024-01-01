//#pragma once
//
//#include "RaytracePass.h"
//
//class AOPass : public RaytracePass
//{
//public:
//	AOPass(Graphics& g, const std::string& name, FrameGraph* parent);
//	virtual void Run(Graphics& g) override;
//	virtual void ShowGUI() override;
//	virtual void Compile(Graphics& g) override;
//
//private:
//	Shared<ConstantBuffer> m_Constants;
//	UINT m_FrameCount;
//};