//#pragma once
//
//#include "ScreenPass.h"
//
//class ToneMapPass : public ScreenPass
//{
//public:
//	ToneMapPass(Graphics& g, const std::string& name, FrameGraph* parent);
//	virtual void Run(Graphics& g) override;
//	virtual void ShowGUI() override;
//
//private:
//	SamplerHeap m_SamplerHeap;
//	Shared<ConstantBuffer> m_Options;
//};