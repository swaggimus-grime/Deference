#include "UI.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

namespace UI
{
	namespace
	{
		ComPtr<ID3D12DescriptorHeap> m_UIHeap;
	}

	void Init()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		ImGui::StyleColorsDark();
	}

	void InitGraphics(Graphics& g)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HR g.Device().CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_UIHeap));
		
		ImGui_ImplDX12_Init(&g.Device(), Graphics::s_NumInFlightFrames, Swapchain::s_Format, m_UIHeap.Get(),
			m_UIHeap->GetCPUDescriptorHandleForHeapStart(), m_UIHeap->GetGPUDescriptorHandleForHeapStart());
	}

	void Shutdown()
	{
		ImGui::DestroyContext();
	}

	void BeginFrame(Graphics& g)
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX12_NewFrame();
		ImGui::NewFrame();
		//ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		ID3D12DescriptorHeap* heaps[] = { m_UIHeap.Get() };
		g.CL().SetDescriptorHeaps(1, heaps);
	}

	void EndFrame(Graphics& g)
	{
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), &g.CL());
	}
}