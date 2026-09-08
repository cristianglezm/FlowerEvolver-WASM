#ifndef FE_TESTS_HELPERS_HPP
#define FE_TESTS_HELPERS_HPP

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <vector>

namespace fe_tests{
	/**
	 * @brief Compares two byte vectors (e.g. fe::Image::imageData) without
	 * ever handing the raw vectors to a Catch2 assertion macro.
	 *
	 * Do NOT write REQUIRE(a == b) for anything image-sized (tens of
	 * thousands of bytes): Catch2 has to be able to stringify both sides
	 * for its report, whether the assertion fails *or* -- if the suite is
	 * run with -s/--success -- passes, and its container stringifier chokes
	 * on inputs this large (throws std::length_error building the display
	 * string, e.g. "basic_string::_M_create"). Catch2 catches that as an
	 * "unexpected exception" and reports the assertion as FAILED even when
	 * the underlying comparison was true -- reproducible locally with
	 * `fe_tests -s "<any test comparing two full-size imageData vectors>"`.
	 * This only ever hands Catch2 two small integers (a size_t and a bool),
	 * so it's safe under -s and gives a more useful failure message besides.
	 */
	inline void requireBytesEqual(const std::vector<std::uint8_t>& a, const std::vector<std::uint8_t>& b){
		REQUIRE(a.size() == b.size());
		std::size_t firstDiff = a.size();
		std::size_t diffCount = 0;
		for(std::size_t i = 0; i < a.size(); ++i){
			if(a[i] != b[i]){
				if(diffCount == 0){
					firstDiff = i;
				}
				++diffCount;
			}
		}
		INFO("first differing byte: " << firstDiff << " (" << diffCount << " of " << a.size() << " bytes differ)");
		REQUIRE(diffCount == 0);
	}
}

#endif // FE_TESTS_HELPERS_HPP
