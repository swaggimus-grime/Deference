#include "Camera.h"

Camera::Camera(Graphics& g, const XMFLOAT3& pos)
	:m_Pos(pos), m_Proj(XMMatrixPerspectiveLH(1.f, 9 / 16.f, 0.5f, 400.0f)), m_MoveSpeed(1.f), m_LookSpeed(0.006f),
	m_Pitch(0.f), m_Yaw(0.f)
{	
	g.CreateBuffer(m_Res, sizeof(CamData), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
	
}

void Camera::Bind(Graphics& g)
{
	g.CL().SetGraphicsRootConstantBufferView(0, m_Res->GetGPUVirtualAddress());
}

void Camera::Update() const
{
	const auto look = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f)
	);

	const auto pos = XMLoadFloat3(&m_Pos);	
	CamData camData;
	camData.view = XMMatrixLookAtLH(pos, pos + look, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	camData.proj = m_Proj;
	XMVECTOR det;
	camData.viewI = XMMatrixInverse(&det, camData.view);
	camData.projI = XMMatrixInverse(&det, camData.proj);
	camData.view = XMMatrixTranspose(camData.view);
	camData.proj = XMMatrixTranspose(camData.proj);

	uint8_t* data;
	HR m_Res->Map(0, nullptr, reinterpret_cast<void**>(&data));
	std::memcpy(data, &camData, sizeof(camData));
	m_Res->Unmap(0, nullptr);
}

void Camera::Rotate(float dx, float dy)
{
	static constexpr float cmf = 2 * M_PI;
	const float mod = fmod(m_Yaw + dx * m_LookSpeed, cmf);
	if (mod > M_PI)
		m_Yaw = mod - cmf;
	else if (mod < -M_PI)
		m_Yaw = mod + cmf;
	else
		m_Yaw = mod;

	m_Pitch = std::clamp(static_cast<double>(m_Pitch + dy * m_LookSpeed), 0.995f * -M_PI / 2.0f, 0.995f * M_PI / 2.0f);
}

void Camera::Move(const XMFLOAT3& delta)
{
	XMFLOAT3 newDelta;
	XMStoreFloat3(&newDelta, XMVector3Transform(
		XMLoadFloat3(&delta),
		XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f) *
		XMMatrixScaling(m_MoveSpeed, m_MoveSpeed, m_MoveSpeed)
	));

	m_Pos += newDelta;
}
