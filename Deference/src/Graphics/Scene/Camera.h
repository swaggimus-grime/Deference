#pragma once

#include "util.h"

namespace Def
{
	class Graphics;

	class Camera : public std::enable_shared_from_this<Camera>
	{
	public:
		Camera(Graphics& g, const XMFLOAT3& pos = { 0.f, 0.f, -1.f });

		void Update();
		void OnResize(UINT w, UINT h);

		inline void SetMoveSpeed(float speed) { m_MoveSpeed = speed; }

		inline XMFLOAT3 Pos() const { return m_Pos; }
		inline float Pitch() const { return m_Pitch; }
		inline float Yaw() const { return m_Yaw; }

		inline XMMATRIX View() const { return m_View; }
		inline XMMATRIX ViewInv() const { return m_ViewInv; }
		inline XMMATRIX Proj() const { return m_Proj; }
		inline XMMATRIX ProjInv() const { return m_ProjInv; }
		inline XMMATRIX WorldToProj() const { return m_View * m_Proj; }
		inline XMMATRIX ProjToWorld() const { return XMMatrixInverse(nullptr, m_View * m_Proj); }
		inline auto U() const { return m_U; }
		inline auto V() const { return m_V; }
		inline auto W() const { return m_W; }

		inline auto Share() const { return shared_from_this(); }
		void Rotate(float dx, float dy);
		void Move(const XMFLOAT3& delta);

		void ShowUI();

	private:
		XMMATRIX m_View;
		XMMATRIX m_ViewInv;
		XMMATRIX m_Proj;
		XMMATRIX m_ProjInv;

		float m_FocalLength = 21.f;
		float m_FocalDistance = 1000.f;
		float m_FrameHeight = 24.f;
		float m_AspectRatio = 1.77f;
		XMFLOAT2 m_Jitter{ 0, 0 };
		XMVECTOR m_U;
		XMVECTOR m_V;
		XMVECTOR m_W;

		XMFLOAT3 m_Pos;
		float m_Pitch;
		float m_Yaw;
		float m_MoveSpeed;
		float m_LookSpeed;
	};
}