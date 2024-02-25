#pragma once

namespace Def
{
	using HCPU = D3D12_CPU_DESCRIPTOR_HANDLE;
	using HGPU = D3D12_GPU_DESCRIPTOR_HANDLE;

	static HCPU operator+(HCPU h1, HCPU h2)
	{
		return { h1.ptr + h2.ptr };
	}

	static HCPU operator-(HCPU h1, HCPU h2)
	{
		return { h1.ptr - h2.ptr };
	}

	static HCPU operator+(HCPU hcpu, UINT offset)
	{
		return { hcpu.ptr + offset };
	}

	static HCPU operator-(HCPU hcpu, UINT offset)
	{
		return { hcpu.ptr - offset };
	}

	static HGPU operator+(HGPU hgpu, UINT offset)
	{
		return { hgpu.ptr + offset };
	}

	static HGPU operator-(HGPU hgpu, UINT offset)
	{
		return { hgpu.ptr - offset };
	}
}