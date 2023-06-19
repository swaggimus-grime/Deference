#pragma once

class Graphics;

class Camera
{
public:
	Camera(Graphics& g, const XMFLOAT3& pos = {0.f, 0.f, -1.f});
	void Bind(Graphics& g);
	void Update() const;
	void Rotate(float dx, float dy);
	void Move(const XMFLOAT3& delta);
	inline ID3D12Resource* Res() const { return m_Res.Get(); }

public:
	struct CamData
	{
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX viewI;
		XMMATRIX projI;
	};

private:
	ComPtr<ID3D12Resource> m_Res;

	XMMATRIX m_Proj;
	XMFLOAT3 m_Pos;
	float m_Pitch;
	float m_Yaw;
	float m_MoveSpeed;
	float m_LookSpeed;
};