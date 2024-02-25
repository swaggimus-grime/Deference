#pragma once

namespace Def
{
	class Model;
	class Camera;

	struct Scene
	{
		Shared<Model> m_Model;
		Shared<Camera> m_Camera;
	};
}