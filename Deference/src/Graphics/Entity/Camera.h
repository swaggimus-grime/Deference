#pragma once

#include "Bindable/Bindable.h"
#include "util.h"

class Graphics;

class Camera : public std::enable_shared_from_this<Camera>
{
public:
	Camera(Graphics& g, const XMFLOAT3& pos = {0.f, 0.f, -1.f});

	void Update();
	void OnResize(UINT w, UINT h);

	inline XMFLOAT3 Pos() const { return m_Pos; }
	inline float Pitch() const { return m_Pitch; }
	inline float Yaw() const { return m_Yaw; }

	inline XMMATRIX View() const { return m_View; }
	inline XMMATRIX ViewInv() const { return m_ViewInv; }
	inline XMMATRIX Proj() const { return m_Proj; }
	inline XMMATRIX ProjInv() const { return m_ProjInv; }

	inline auto Share() const { return shared_from_this(); }
	void Rotate(float dx, float dy);
	void Move(const XMFLOAT3& delta);

	void ShowUI();

private:
	XMMATRIX m_View;
	XMMATRIX m_ViewInv;
	XMMATRIX m_Proj;
	XMMATRIX m_ProjInv;

	XMFLOAT3 m_Pos;
	float m_Pitch;
	float m_Yaw;
	float m_MoveSpeed;
	float m_LookSpeed;
};