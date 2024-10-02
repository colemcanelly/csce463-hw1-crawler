// pch.h: This is a precompiled header file.
/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// Uncomment this to test HTTP1.1 chunking functionality
//#define HTTP1_1

// Uncomment this to count the In-Degree of TAMU.edu
//#define GET_IN_TAMU

// add headers that you want to pre-compile here
#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include <optional>
#include <limits>
#include <exception>
#include <chrono>
#include <set>
#include <mutex>
#include <functional>

#include "HTMLParserBase.h"

#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

// Timing operations
using Time = std::chrono::high_resolution_clock;
using seconds = std::chrono::seconds;
using milliseconds = std::chrono::milliseconds;

template <typename T>
constexpr T time_elapsed(Time::time_point begin) { return std::chrono::duration_cast<T>(Time::now() - begin); }

// Unit conversion

// Bytes per bit
using bytes = std::ratio<8, 1>;

template <typename ToRatio, typename FromRatio = std::ratio<1>, typename T>
constexpr T unit_cast(T value) {
	using conversion_ratio = std::ratio_divide<FromRatio, ToRatio>;
	return value * static_cast<T>(conversion_ratio::num) / static_cast<T>(conversion_ratio::den);
}


// Operator literals
using std::chrono::operator""s;
using std::chrono::operator""ms;

constexpr size_t operator""KB(const size_t x) { return x * 1000; }
constexpr size_t operator""MB(const size_t x) { return x * 1000KB; }

constexpr size_t operator""KiB(const size_t x) { return x << 10; }
constexpr size_t operator""MiB(const size_t x) { return x << 20; }

namespace UInt {
	template <typename T>
	inline std::optional<T> parse(const std::string& s, int base = 10)
	try {
		uint64_t ret = std::strtoull(s.data(), nullptr, base);
		if (ret > std::numeric_limits<T>::max()) return {};
		return static_cast<T>(ret);
	}
	catch (...) {
		return {};
	}
}

#define USAGE_INFO \
	"\n\nUsage: crawler [<url> | <num_threads> <url_file.txt>]" "\n" \
	"- <num_threads> must always be = 1"						"\n" \
	"- <url> = scheme://host[:port][/path][?query][#fragment]""\n" \
	"\t-> scheme must always be \"http\""					"\n" \
	"\t-> port must be a valid 16-bit unsigned integer"

#endif //PCH_H