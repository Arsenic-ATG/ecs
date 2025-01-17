#ifndef ECS_ENTITY_RANGE_ITERATOR
#define ECS_ENTITY_RANGE_ITERATOR

#include "../entity_range.h"
#include "contract.h"
#include "entity_iterator.h"

#include <iterator>
#include <limits>

namespace ecs::detail {

// Iterate over a span of entity_ranges, producing each individual entity
class entity_range_iterator {
public:
	// iterator traits
	using difference_type = ptrdiff_t;
	using value_type = entity_type;
	using pointer = const entity_type*;
	using reference = const entity_type&;
	using iterator_category = std::random_access_iterator_tag;

	constexpr entity_range_iterator() noexcept {};

	constexpr entity_range_iterator(entity_range_view _ranges) noexcept : ranges(_ranges) {
		if (ranges.size() > 0) {
			range_it = ranges.front().begin();
			range_end = ranges.front().end();
		}
	}

	constexpr entity_range_iterator& operator++() {
		step(1);
		return *this;
	}

	constexpr entity_range_iterator operator++(int) {
		entity_range_iterator const retval = *this;
		++(*this);
		return retval;
	}

	constexpr entity_range_iterator operator+(difference_type diff) const {
		auto copy = *this;
		copy.step(diff);
		return copy;
	}

	// Operator exclusively for GCC. This operator is called by GCC's parallel implementation
	// for some goddamn reason. When has this ever been a thing?
	constexpr value_type operator[](int index) const {
		return *(*this + index);
	}

	// returns the distance between two iterators
	/*constexpr value_type operator-(entity_range_iterator other) const {
		return {};
		//step(ent_, -other.ent_);
	}*/

	constexpr bool operator==(entity_range_iterator other) const {
		// Comparison with end iterator
		if (is_at_end())
			return other.is_at_end();
		if (other.is_at_end())
			return is_at_end();

		// Can't compare iterators from different sources
		Expects(ranges.data() == other.ranges.data());

		return current_range_index == other.current_range_index && range_it == other.range_it;
	}

	constexpr bool operator!=(entity_range_iterator other) const {
		return !(*this == other);
	}

	constexpr entity_id operator*() {
		return *range_it;
	}

protected:
	constexpr bool is_at_end() const {
		return current_range_index == ranges.size();
	}

	// Step forward 'diff' entities.
	constexpr void step(ptrdiff_t diff) {
		auto const current_range_dist = std::distance(range_it, range_end);
		if (diff < current_range_dist) {
			// Simple step forward in the current range
			range_it = range_it + diff;

		} else {
			// The step spans more than one range

			auto remainder = diff - current_range_dist;

			// Go to the next range
			current_range_index += 1;

			// If I have hit the end, this is now an end-iterator
			if (current_range_index == ranges.size()) {
				return;
			}

			// Find the range
			auto const count = static_cast<ptrdiff_t>(ranges[current_range_index].count());
			while (remainder >= count) {
				// Update the remainder
				remainder -= count;

				current_range_index += 1;
				if (current_range_index == ranges.size()) {
					return;
				}
			};

			// Update the iterators
			range_it = ranges[current_range_index].begin() + remainder;
			range_end = ranges[current_range_index].end();
		}
	}

private:
	// All the ranges
	entity_range_view ranges;

	// Iterators into a specific range
	entity_iterator range_it;
	entity_iterator range_end;

	// The range currently being iterated
	size_t current_range_index = 0;
};

} // namespace ecs::detail

#endif // !ECS_ENTITY_RANGE_ITERATOR
