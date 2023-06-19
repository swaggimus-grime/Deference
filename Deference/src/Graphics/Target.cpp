#include "Target.h"

Target::Target(D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES state, ComPtr<ID3D12Resource> res)
	:Resource(handle, state, res)
{
}
