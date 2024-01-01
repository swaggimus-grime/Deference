//#pragma once
//
//#include "RaytracePass.h"
//
//class DiffusePass : public RaytracePass
//{
//public:
//	DiffusePass(Graphics& g, const std::string& name, FrameGraph* parent);
//	virtual void Run(Graphics& g) override;
//	virtual void ShowGUI() override;
//	virtual void Finish(Graphics& g) override;
//
//private:
//	Shared<ConstantBuffer> m_Light;
//};