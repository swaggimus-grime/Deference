#include "Camera.h"
#include <numbers>
#include <imgui.h>
#include "VecOps.h"
#include "util.h"

namespace Def
{
	constexpr float pi = std::numbers::pi_v<float>;

	Camera::Camera(Graphics& g, const XMFLOAT3& pos)
		:m_HashChanged(false), m_Pos(pos), m_LookSpeed(0.006f), m_MoveSpeed(0.1f),
		m_Pitch(0.f), m_Yaw(0.f)
	{
		Update();
		OnResize(g.Width(), g.Height());
	}

	inline float focalLengthToFovY(float focalLength, float frameHeight)
	{
		return 2.0f * atan(0.5f * frameHeight / focalLength);
	}

	void Camera::Update()
	{
		m_HashChanged = false;

		const auto look = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
			XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f)
		);
		const auto pos = XMLoadFloat3(&m_Pos);
		m_View = XMMatrixLookAtLH(pos, XMVectorAdd(pos, look), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		m_ViewInv = XMMatrixInverse(nullptr, m_View);

		// Interpret focal length of 0 as 0 FOV. Technically 0 FOV should be focal length of infinity.
		const float fovY = m_FocalLength == 0.0f ? 0.0f : focalLengthToFovY(m_FocalLength, m_FrameHeight);

		//// Build jitter matrix
		//// (jitterX and jitterY are expressed as subpixel quantities divided by the screen resolution
		////  for instance to apply an offset of half pixel along the X axis we set jitterX = 0.5f / Width)
		//XMMATRIX jitterMat(1.0f, 0.0f, 0.0f, 0.0f,
		//    0.0f, 1.0f, 0.0f, 0.0f,
		//    0.0f, 0.0f, 1.0f, 0.0f,
		//    2.0f * m_Jitter.x, 2.0f * m_Jitter.y, 0.0f, 1.0f);
		//// Apply jitter matrix to the projection matrix
		//m_Proj = m_Proj * jitterMat;

		// Ray tracing related vectors
		m_W = XMVectorScale(XMVector3Normalize(look), m_FocalDistance);
		m_U = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), m_W));
		m_V = XMVector3Normalize(XMVector3Cross(m_W, m_U));
		const float ulen = m_FocalDistance * tanf(fovY * 0.5f) * m_AspectRatio;
		m_U *= ulen;
		const float vlen = m_FocalDistance * tanf(fovY * 0.5f);
		m_V *= vlen;
	}

	void Camera::OnResize(UINT w, UINT h)
	{
		m_Proj = XMMatrixPerspectiveFovLH(pi / 2.f, static_cast<FLOAT>(w) / h, 0.001f, 4000.0f);
		m_ProjInv = XMMatrixInverse(nullptr, m_Proj);
	}

	void Camera::Rotate(float dx, float dy)
	{
		m_HashChanged = true;

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
		m_HashChanged = true;

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
			ImGui::SliderFloat("Move Speed", &m_MoveSpeed, 0.1f, 10.f, "%.1f");
			m_HashChanged |= ImGui::SliderFloat("Focal Length", &m_FocalLength, 0.1f, 100.f, "%.1f");
			m_HashChanged |= ImGui::SliderFloat("Focal Distance", &m_FocalDistance, 1.f, 10000.f, "%.9f");
			m_HashChanged |= ImGui::SliderFloat("Frame Height", &m_FrameHeight, 0.1f, 100.f, "%.1f");
			m_HashChanged |= ImGui::SliderFloat("Aspect Ratio", &m_AspectRatio, 0.1f, 3.f, "%.1f");
		ImGui::End();
	}

}