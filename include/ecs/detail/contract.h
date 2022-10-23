#ifndef ECS_DETAIL_CONTRACT_H
#define ECS_DETAIL_CONTRACT_H

#ifdef NDEBUG
#ifdef __has_cpp_attribute                  // Check if __has_cpp_attribute is present
#  if __has_cpp_attribute(assume)           // Check for an attribute
     // Contracts. If they are violated in release, assume 'cond' is correct
#    define Expects(cond) [[assume(cond)]]
#    define Ensures(cond) [[assume(cond)]]
#  else
#    define Expects(cond) static_cast<void>((cond))
#    define Ensures(cond) static_cast<void>((cond))
#  endif
#endif

#else
 // Contracts. If they are violated in debug, the program is an invalid state, so nuke it from orbit
#define Expects(cond)                                                                                                                      \
	do {                                                                                                                                   \
		((cond) ? static_cast<void>(0) : std::terminate());                                                                                \
	} while (false)
#define Ensures(cond)                                                                                                                      \
	do {                                                                                                                                   \
		((cond) ? static_cast<void>(0) : std::terminate());                                                                                \
	} while (false)
#endif // NDEBUG

#endif // !ECS_DETAIL_CONTRACT_H
