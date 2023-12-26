#include "UI.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

namespace UI
{
	namespace
	{
		Unique<GPUShaderHeap> m_UIHeap;
		Unique<RenderTargetHeap> m_TargetHeap;
		Shared<RenderTarget> m_Target;
		HGPU m_TargetHandle;
	}

	void Init()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		ImGui::StyleColorsDark();
	}

	void InitGraphics(Graphics& g)
	{
		m_UIHeap = MakeUnique<GPUShaderHeap>(g, 2);
		m_TargetHeap = MakeUnique<RenderTargetHeap>(g, 1);
		
		ImGui_ImplDX12_Init(&g.Device(), Graphics::s_NumInFlightFrames, Swapchain::s_Format, **m_UIHeap,
			m_UIHeap->CPUStart(), m_UIHeap->GPUStart());

		m_Target = MakeShared<RenderTarget>(g);
		m_TargetHeap->Add(g, m_Target);
		m_UIHeap->IncrementHandle();
		m_TargetHandle = m_UIHeap->AddTarget(g, m_Target);

		m_Target->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void Shutdown()
	{
		ImGui::DestroyContext();
	}

	void OnResize(Graphics& g, UINT width, UINT height)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(width, height);
		m_Target->Resize(g, width, height);
		m_Target->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void BeginFrame(Graphics& g)
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX12_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ID3D12DescriptorHeap* heaps[] = { **m_UIHeap };
		g.CL().SetDescriptorHeaps(1, heaps);
	}

	void EndFrame(Graphics& g)
	{
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), &g.CL());
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void DrawTarget(Graphics& g, Shared<Target> target)
	{
		m_Target->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		target->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

		g.CL().CopyResource(**m_Target, **target);

		m_Target->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		target->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

		auto pos = ImGui::GetWindowPos();
		ImGui::GetWindowDrawList()->AddImage(
			(ImTextureID)m_TargetHandle.ptr, pos, {pos.x + g.Width(), pos.y + g.Height()},
			ImVec2(0, 0), ImVec2(1, 1));
	}
}