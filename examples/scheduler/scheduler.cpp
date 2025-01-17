#include <ecs/ecs.h>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

// A small example that creates 6 systems with dependencies on 3 components.
//
// Prints out each systems dependencies, which
// can then be verified when the systems are run.
//
// Systems without dependencies are executed concurrently,
// while systems with dependencies will only be executed
// after other systems are done with them.

template <size_t I>
struct type {};

int main() {
	using namespace ecs::opts;
	ecs::runtime ecs;

	std::cout << std::boolalpha;
	std::cout << "creating systems:\n";

	//
	// Assumes that type 0 is writte to, and type 1 is only read from.
	auto &sys1 = ecs.make_system<manual_update>([](type<0> &, type<1> const &) {
		std::cout << "1 ";
		std::this_thread::sleep_for(20ms); // simulate work
	});
	std::cout << "\nsys1 (type<0>&, type<1> const&)\n";

	//
	// Writes to type 1. This system must not execute until after sys1 is done,
	// in order to avoid race conditions.
	auto &sys2 = ecs.make_system<manual_update>([](type<1> &) {
		std::cout << "2 ";
		std::this_thread::sleep_for(20ms);
	});
	std::cout << "\nsys2 (type<1>&)\n";
	std::cout << " depends on 1? " << sys2.depends_on(&sys1) << '\n';

	//
	// Writes to type 2. This has no dependencies on type 0 or 1, so it can be run
	// concurrently with sys1 and sys2.
	auto &sys3 = ecs.make_system<manual_update>([](type<2> &) {
		std::cout << "3 ";
		std::this_thread::sleep_for(20ms);
	});
	std::cout << "\nsys3 (type<2>&)\n";
	std::cout << " depends on 1? " << sys3.depends_on(&sys1) << '\n';
	std::cout << " depends on 2? " << sys3.depends_on(&sys2) << '\n';

	//
	// Reads from type 0. Must not execute until sys1 is done.
	auto &sys4 = ecs.make_system<manual_update>([](type<0> const &) {
		std::cout << "4 ";
		std::this_thread::sleep_for(20ms);
	});
	std::cout << "\nsys4 (type<0> const&)\n";
	std::cout << " depends on 1? " << sys4.depends_on(&sys1) << '\n';
	std::cout << " depends on 2? " << sys4.depends_on(&sys2) << '\n';
	std::cout << " depends on 3? " << sys4.depends_on(&sys3) << '\n';

	//
	// Writes to type 2 and reads from type 0. Must not execute until after
	// sys3 and sys1 us done.
	auto &sys5 = ecs.make_system<manual_update>([](type<2> &, type<0> const &) {
		std::cout << "5 ";
		std::this_thread::sleep_for(20ms);
	});
	std::cout << "\nsys5 (type<2>&, type<0> const&)\n";
	std::cout << " depends on 1? " << sys5.depends_on(&sys1) << '\n';
	std::cout << " depends on 2? " << sys5.depends_on(&sys2) << '\n';
	std::cout << " depends on 3? " << sys5.depends_on(&sys3) << '\n';
	std::cout << " depends on 4? " << sys5.depends_on(&sys4) << '\n';

	//
	// Reads from type 2. Must not execute until sys5 is done.
	auto &sys6 = ecs.make_system<manual_update>([](type<2> const &) {
		std::cout << "6 ";
		std::this_thread::sleep_for(20ms);
	});
	std::cout << "\nsys6 (type<2> const&)\n";
	std::cout << " depends on 1? " << sys6.depends_on(&sys1) << '\n';
	std::cout << " depends on 2? " << sys6.depends_on(&sys2) << '\n';
	std::cout << " depends on 3? " << sys6.depends_on(&sys3) << '\n';
	std::cout << " depends on 4? " << sys6.depends_on(&sys4) << '\n';
	std::cout << " depends on 5? " << sys6.depends_on(&sys5) << '\n';

	//
	// Add the components to an entitiy and run the systems.
	std::cout << "\nrunning systems on 5 entities:\n";
	ecs.add_component({0, 4}, type<0>{}, type<1>{}, type<2>{});
	ecs.update();
	std::cout << '\n';
}
