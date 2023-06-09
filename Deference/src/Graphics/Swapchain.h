#pragma once

#include <dxgi1_6.h>
#include <wrl.h>
#include "Bindable/Heap/Heap.h"
#include "Bindable/Heap/RenderTarget.h"

class Swapchain
{
public:
	Swapchain(class Graphics& g, HWND hWnd, UINT numBuffs);
	void Present();
	inline UINT NumBuffs() const { return m_NumBuffs; }
	inline IDXGISwapChain4* Handle() const { return m_SC.Get(); }
	inline Shared<RenderTarget> CurrentBB() const
	{
		UINT currentIdx = m_SC->GetCurrentBackBufferIndex();
		return m_BackBuffs[currentIdx];
	}

private:
	std::vector<Shared<RenderTarget>> m_BackBuffs;
	Unique<RenderTargetHeap> m_RTs;
	ComPtr<IDXGISwapChain4> m_SC;
	UINT m_NumBuffs;
	bool m_AllowTearing;
};