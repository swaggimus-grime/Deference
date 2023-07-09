#pragma once

#include "Bindable/Bindable.h"
#include "util.h"

class Graphics;

class Camera : public std::enable_shared_from_this<Camera>
{
public:
	Camera(Graphics& g, const XMFLOAT3& pos = {0.f, 0.f, -1.f});

	inline XMFLOAT3 Pos() const { return m_Pos; }

	XMMATRIX View() const;
	inline XMMATRIX Proj() const { return m_Proj; }

	inline auto Share() const { return shared_from_this(); }
	void Rotate(float dx, float dy);
	void Move(const XMFLOAT3& delta);

private:
	XMMATRIX m_Proj;

	XMFLOAT3 m_Pos;
	float m_Pitch;
	float m_Yaw;
	float m_MoveSpeed;
	float m_LookSpeed;
};