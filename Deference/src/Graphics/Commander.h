#pragma once

#include <vector>

namespace Def
{
	template<class C>
	class Commander
	{
	public:
		static Commander& Init()
		{
			m_Instance = MakeUnique<Commander>();
			m_Chainables.clear();
			return *m_Instance;
		}

		Commander& Add(const C& c)
		{
			m_Chainables.push_back(c);
			return *m_Instance;
		}

		template<class F>
		void Bind(Graphics& g)
		{
			F::Bind(g, m_Chainables);
			m_Instance.release();
		}

		void Transition(Graphics& g)
		{
			Resource::Transition(g, m_Chainables);
			m_Instance.release();
		}

		Commander() = default;

	private:
		static Unique<Commander> m_Instance;
		static std::vector<C> m_Chainables;
	};

	template<class C>
	Unique<Commander<C>> Commander<C>::m_Instance = nullptr;

	template<class C>
	std::vector<C> Commander<C>::m_Chainables = {};
}