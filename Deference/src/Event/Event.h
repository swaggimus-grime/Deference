#pragma once

#include <queue>
#include <functional>
#include <concepts>
#include <typeinfo>
#include <unordered_set>
#include "util.h"

namespace Def
{
	class Event
	{
	public:

	protected:
		Event() = default;
	};

	class EventManager
	{
	private:
		template<class E>
			requires Derived<Event, E>
		using EventFn = std::function<void(E&)>;

		template<class E>
			requires Derived<Event, E>
		struct EventFunction
		{
			EventFn<E> m_Func;

			EventFunction(EventFn<E> func)
				:m_Func(func)
			{}

			void operator()(E& e)
			{
				m_Func(e);
			}

			bool operator==(const EventFunction<E>& e) const
			{
				return typeid(this) == typeid(e);
			}
		};

		template<class E>
			requires Derived<Event, E>
		struct EventHash
		{
			size_t operator()(const EventFunction<E>& e) const noexcept
			{
				return typeid(e).hash_code();
			}
		};

	public:
		template<class E, typename ... Args>
			requires Derived<Event, E>
		static void Execute(Args&& ... args)
		{
			E e(std::forward<Args>(args)...);
			std::unordered_set<EventFunction<E>, EventHash<E>>& bindings = m_Bindings<E>[std::string(typeid(E).name())];
			for (EventFunction<E> b : bindings)
				b(e);
		}

		template<class E>
			requires Derived<Event, E>
		static void Bind(EventFn<E> fn)
		{
			std::unordered_set<EventFunction<E>, EventHash<E>>& set = m_Bindings<E>[std::string(typeid(E).name())];
			EventFunction<E> e(fn);
			set.insert(e);
		}

	private:
		EventManager() = delete;

	private:
		template<class E>
			requires Derived<Event, E>
		static std::map<std::string, std::unordered_set<EventFunction<E>, EventHash<E>>> m_Bindings;
		
	};

	template<class E>
		requires Derived<Event, E>
	std::map<std::string, std::unordered_set<EventManager::EventFunction<E>, EventManager::EventHash<E>>> EventManager::m_Bindings;

#define EVENT_BIND(fn) std::bind(&fn, this, std::placeholders::_1)
}