#include "Camera.h"
#include <numbers>
#include <imgui.h>

constexpr float pi = std::numbers::pi_v<float>;

Camera::Camera(Graphics& g, const XMFLOAT3& pos)
	:m_Pos(pos), m_LookSpeed(0.006f), m_MoveSpeed(1.f),
	m_Pitch(0.f), m_Yaw(0.f)
{
	Update();
	OnResize(g.Width(), g.Height());
}

void Camera::Update()
{
	const auto look = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f)
	);
	const auto pos = XMLoadFloat3(&m_Pos);	
	m_View = XMMatrixLookAtLH(pos, pos + look, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	m_ViewInv = XMMatrixInverse(nullptr, m_View);

	auto dirDU = XMVector3Normalize(XMVector3Cross(look, XMVectorSet(0, 1, 0, 0))) * m_ImgPlaneSize.x;
	XMStoreFloat3(&m_DirDU, dirDU);
	auto dirDV = XMVector3Normalize(XMVector3Cross(dirDU, look)) * m_ImgPlaneSize.y;
	XMStoreFloat3(&m_DirDV, dirDV);
	XMStoreFloat3(&m_DirTopLeft, look - 0.5f * dirDU - 0.5f * dirDV);
}

void Camera::OnResize(UINT w, UINT h)
{
	m_Proj = XMMatrixPerspectiveFovLH(pi / 2.f, static_cast<FLOAT>(w) / h, 0.001f, 4000.0f);
	m_ProjInv = XMMatrixInverse(nullptr, m_Proj);

	m_ImgPlaneSize.y = 2.f * std::tan(0.5f * pi / 2.f);
	m_ImgPlaneSize.x = m_ImgPlaneSize.y * static_cast<float>(w) / h;
}

void Camera::Rotate(float dx, float dy)
{
	static constexpr float cmf = 2 * pi;
	const float mod = fmod(m_Yaw + dx * m_LookSpeed, cmf);
	if (mod > pi)
		m_Yaw = mod - cmf;
	else if (mod < -pi)
		m_Yaw = mod + cmf;
	else
		m_Yaw = mod;

	m_Pitch = std::clamp(static_cast<float>(m_Pitch + dy * m_LookSpeed), 0.995f * -pi / 2.0f, 0.995f * pi / 2.0f);
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

void Camera::ShowUI()
{
	if (ImGui::Begin("Camera"))
	{
		ImGui::SliderFloat("Move Speed", &m_MoveSpeed, 1.f, 100.f);
		ImGui::End();
	}
}
