#include "AccelStruct.h"

namespace Def
{
	TLAS::TLAS(Graphics& g, const std::vector<ComPtr<ID3D12Resource>>& blass)
		:SRV(this)
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		inputs.NumDescs = blass.size();
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
		g.Device().GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		ComPtr<ID3D12Resource> scratch;
		{
			const CD3DX12_HEAP_PROPERTIES heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto res = CD3DX12_RESOURCE_DESC::Buffer(info.ScratchDataSizeInBytes);
			res.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			HR g.Device().CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &res, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&scratch));
		}

		{
			const CD3DX12_HEAP_PROPERTIES heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto res = CD3DX12_RESOURCE_DESC::Buffer(info.ResultDataMaxSizeInBytes);
			res.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			HR g.Device().CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &res, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&m_Res));
			m_Res->SetName(L"TLAS");
		}

		ComPtr<ID3D12Resource> instances;
		{
			const CD3DX12_HEAP_PROPERTIES heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto res = CD3DX12_RESOURCE_DESC::Buffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * blass.size());
			res.Flags = D3D12_RESOURCE_FLAG_NONE;
			HR g.Device().CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &res, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&instances));
		}

		D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs;
		instances->Map(0, nullptr, reinterpret_cast<void**>(&instanceDescs));
		SecureZeroMemory(instanceDescs, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * blass.size());
		for (UINT i = 0; i < blass.size(); i++)
		{
			instanceDescs[i].InstanceID = i;
			instanceDescs[i].InstanceContributionToHitGroupIndex = i;
			instanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			XMMATRIX mat = XMMatrixIdentity();
			std::memcpy(instanceDescs[i].Transform, &mat, sizeof(instanceDescs[i].Transform));
			instanceDescs[i].AccelerationStructure = blass[i]->GetGPUVirtualAddress();
			instanceDescs[i].InstanceMask = 0xFF;
		}
		instances->Unmap(0, nullptr);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC as = {};
		as.Inputs = inputs;
		as.Inputs.InstanceDescs = instances->GetGPUVirtualAddress();
		as.DestAccelerationStructureData = m_Res->GetGPUVirtualAddress();
		as.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
		g.CL().BuildRaytracingAccelerationStructure(&as, 0, nullptr);

		const auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_Res.Get());
		g.CL().ResourceBarrier(1, &barrier);

		m_Res->SetName(L"TLAS");

		g.Flush();
	}

	ComPtr<ID3D12Resource> TLAS::BLAS(Graphics& g, const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometries)
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		inputs.NumDescs = geometries.size();
		inputs.pGeometryDescs = geometries.data();
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
		g.Device().GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		ComPtr<ID3D12Resource> scratch;
		{
			const CD3DX12_HEAP_PROPERTIES heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto res = CD3DX12_RESOURCE_DESC::Buffer(info.ScratchDataSizeInBytes);
			res.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			HR g.Device().CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &res, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&scratch));
			scratch->SetName(L"BLAS scratch");
		}

		ComPtr<ID3D12Resource> result;
		{
			const CD3DX12_HEAP_PROPERTIES heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto res = CD3DX12_RESOURCE_DESC::Buffer(info.ResultDataMaxSizeInBytes);
			res.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			HR g.Device().CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &res, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&result));
			result->SetName(L"BLAS");
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC as = {};
		as.Inputs = inputs;
		as.DestAccelerationStructureData = result->GetGPUVirtualAddress();
		as.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();

		g.CL().BuildRaytracingAccelerationStructure(&as, 0, nullptr);
		const auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(result.Get());
		g.CL().ResourceBarrier(1, &barrier);

		g.Flush();

		return result;
	}

	const D3D12_SHADER_RESOURCE_VIEW_DESC& TLAS::SRVDesc() const
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.RaytracingAccelerationStructure.Location = GetGPUAddress();
		
		return std::move(desc);
	}
}

