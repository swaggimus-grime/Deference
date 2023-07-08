#include "Target.h"

Target::Target(D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res, D3D12_RESOURCE_STATES state)
	:Resource(handle, res, state)
{
}
