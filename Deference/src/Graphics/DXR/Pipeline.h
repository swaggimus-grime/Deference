#pragma once

#include "Bindable/Bindable.h"
#include "nv_helpers_dx12/RaytracingPipelineGenerator.h"
#include "nv_helpers_dx12/ShaderBindingTableGenerator.h"

class Graphics;
class RootSig;

class DXRParams
{
public:
	friend class DXRPipeline;

	DXRParams(Graphics& g);
	void AddHitGroup(const std::wstring& name, const std::wstring& closestHitSymbol);
	void AddLibrary(const std::wstring& shaderPath, const std::initializer_list<std::wstring>& symbols);
	void AddRootSig(ComPtr<ID3D12RootSignature> sig, const std::initializer_list<std::wstring>& symbols);
	void SetMaxRecursion(UINT max);
	void SetMaxPayloadSize(UINT max);
	void SetMaxAttribSize(UINT max);

private:
	nv_helpers_dx12::RayTracingPipelineGenerator m_Gen;
	std::vector<ComPtr<IDxcBlob>> m_Libs;
	std::vector<ComPtr<ID3D12RootSignature>> m_Sigs;
};

class SBTParams
{
public:
	friend class DXRPipeline;

	void AddRayGenProg(const std::wstring& entryPoint, const std::initializer_list<void*> inputs = {});
	void AddMissProg(const std::wstring& entryPoint, const std::initializer_list<void*> inputs = {});
	void AddHitGroup(const std::wstring& entryPoint, const std::initializer_list<void*> inputs = {});

private:
	nv_helpers_dx12::ShaderBindingTableGenerator m_SBT;
};

class DXRPipeline : public Bindable
{
public:
	DXRPipeline(Graphics& g, DXRParams& pipelineParams, SBTParams& sbtParams);
	virtual void Bind(Graphics& g) override;
	void Dispatch(Graphics& g);

private:
	ComPtr<ID3D12StateObject> m_State;
	ComPtr<ID3D12StateObjectProperties> m_Props;

	std::vector<ComPtr<IDxcBlob>> m_Libs;
	std::vector<ComPtr<ID3D12RootSignature>> m_Sigs;

	ComPtr<ID3D12Resource> m_SBTBuff;
	nv_helpers_dx12::ShaderBindingTableGenerator m_SBT;
};