#pragma once
#include <execution>
#include "system_inspector.h"
#include "entity_range.h"
#include "component_pool.h"
#include "component_specifier.h"
#include "context.h"

namespace ecs {
	// Adds a component to a range of entities. Will not be added until 'commit_changes()' is called.
	// If T is invokable as 'T(entity_id)' the initializer function is called for each entity,
	// and its return type defines the component type
	// Pre: entity does not already have the component, or have it in queue to be added
	template <typename T>
	void add_component(entity_range const range, T val)
	{
		bool constexpr invokable = std::is_invocable_v<T, entity_id>;
		if constexpr (invokable) {
			// Return type of 'init'
			using ComponentType = decltype(val(entity_id{ 0 }));
			static_assert(!std::is_same_v<ComponentType, void>, "Initializer function must have a return value");

			// Add it to the component pool
			detail::component_pool<ComponentType>& pool = detail::_context.get_component_pool<ComponentType>();
			pool.add_init(range, val);
		}
		else {
			static_assert(std::is_copy_constructible_v<T>, "A copy-constructor for type T is required for this function to work");

			// Add it to the component pool
			detail::component_pool<T>& pool = detail::_context.get_component_pool<T>();
			if constexpr (std::is_move_constructible_v<T>)
				pool.add(range, std::move(val));
			else
				pool.add(range, val);
		}
	}

	// Adds a component to an entity. Will not be added until 'commit_changes()' is called.
	// Pre: entity does not already have the component, or have it in queue to be added
	template <typename T>
	void add_component(entity_id const id, T val)
	{
		add_component({ id, id }, std::move(val));
	}

	// Removes a component from a range of entities. Will not be removed until 'commit_changes()' is called.
	// Pre: entity has the component
	template <typename T>
	void remove_component(entity_range const range)
	{
		static_assert(!detail::is_transient_v<T>, "Don't remove transient components manually; it will be handled by the context");

		// Remove the entities from the components pool
		detail::component_pool<T> &pool = detail::_context.get_component_pool<T>();
		pool.remove_range(range);
	}

	// Removes a component from an entity. Will not be removed until 'commit_changes()' is called.
	// Pre: entity has the component
	template <typename T>
	void remove_component(entity_id const id)
	{
		remove_component<T>({ id, id });
	}

	// Removes all components from an entity
	/*inline void remove_all_components(entity_id const id)
	{
		for (auto const& pool : context::internal::component_pools)
			pool->remove(id);
	}*/

	// Returns a shared component. Can be called before a system for it has been added
	template <typename T>
	T& get_shared_component()
	{
		static_assert(detail::is_shared_v<T>, "Component has not been marked as shared. Inherit from ecs::shared to fix this.");

		// Get the pool
		if (!detail::_context.has_component_pool(typeid(T)))
			detail::_context.init_component_pools<T>();
		return detail::_context.get_component_pool<T>().get_shared_component();
	}

	// Returns the component from an entity.
	// Pre: the entity has the component
	template <typename T>
	T& get_component(entity_id const id)
	{
		// Get the component pool
		detail::component_pool<T> const& pool = detail::_context.get_component_pool<T>();
		return pool.find_component_data(id);
	}

	// Returns the number of active components
	template <typename T>
	size_t get_component_count()
	{
		if (!detail::_context.has_component_pool(typeid(T)))
			return 0;

		// Get the component pool
		detail::component_pool<T> const& pool = detail::_context.get_component_pool<T>();
		return pool.num_components();
	}

	// Returns the number of entities that has the component.
	template <typename T>
	size_t get_entity_count()
	{
		if (!detail::_context.has_component_pool(typeid(T)))
			return 0;

		// Get the component pool
		detail::component_pool<T> const& pool = detail::_context.get_component_pool<T>();
		return pool.num_entities();
	}

	// Return true if an entity contains the component
	template <typename T>
	bool has_component(entity_id const id)
	{
		if (!detail::_context.has_component_pool(typeid(T)))
			return false;

		detail::component_pool<T> const& pool = detail::_context.get_component_pool<T>();
		return pool.has_entity(id);
	}

	// Returns true if all entities in a range has the component.
	template <typename T>
	bool has_component(entity_range const range)
	{
		if (!detail::_context.has_component_pool(typeid(T)))
			return false;

		detail::component_pool<T> &pool = detail::_context.get_component_pool<T>();
		return pool.has_entity(range);
	}

	// Commits the changes to the entities.
	inline void commit_changes()
	{
		detail::_context.commit_changes();
	}

	// Calls the 'update' function on all the systems in the order they were added.
	inline void run_systems() noexcept
	{
		detail::_context.run_systems();
	}

	// Commits all changes and calls the 'update' function on all the systems in the order they were added.
	// Same as calling commit_changes() and run_systems().
	inline void update_systems()
	{
		commit_changes();
		run_systems();
	}

	// Add a new system to the context. It will process components in parallel.
	template <typename System>
	// requires Callable<System>
	auto& add_system_parallel(System update_func)
	{
		detail::verify_system<System>();
		return detail::_context.create_system<std::execution::parallel_unsequenced_policy, System>(update_func, &System::operator());
	}

	// Add a new system to the context
	template <typename System>
	// requires Callable<System>
	auto& add_system(System update_func)
	{
		detail::verify_system<System>();
		return detail::_context.create_system<std::execution::sequenced_policy, System>(update_func, &System::operator());
	}
}