#pragma once

class Model;
class Camera;

class Scene
{
public:

	inline void SetModel(const Shared<Model>& model) { m_Model = model; }
	inline auto& GetModel() const { return *m_Model; }

	inline void SetCamera(const Shared<Camera>& cam) { m_Camera = cam; }
	inline auto& GetCamera() const { return *m_Camera; }

private:
	Shared<Model> m_Model;
	Shared<Camera> m_Camera;
};