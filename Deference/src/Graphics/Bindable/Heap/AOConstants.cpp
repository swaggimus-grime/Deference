#include "AOConstants.h"
#include "Graphics.h"

AOConstants::AOConstants(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle)
	:ConstantBuffer(g, handle)
{
	Update(0);
}

void AOConstants::Update(UINT frameCount)
{
	AOConstantsParams p{};
	p.frameCount = frameCount;
	UpdateStruct(&p);
}