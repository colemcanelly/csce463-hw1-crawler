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

#include "HTMLParserBase.h"

#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>


#define TIME_CURRENT std::chrono::high_resolution_clock::now()
#define TIME_ELAPSED(begin) std::chrono::duration_cast<std::chrono::milliseconds>(TIME_CURRENT - begin)

constexpr size_t operator""KB(const size_t x) { return x * 1000; }
constexpr size_t operator""MB(const size_t x) { return x * 1000KB; }

constexpr size_t operator""KiB(const size_t x) { return x * 1024; }
constexpr size_t operator""MiB(const size_t x) { return x * 1024KiB; }

namespace UInt {
	template <typename T>
	inline std::optional<T> parse(const std::string& s) {
		try {
			uint64_t port = std::stoull(s);
			if (port > std::numeric_limits<T>::max() || port == 0) return {};
			return static_cast<T>(port);
		}
		catch (...) {
			return {};
		}
	}
}

#define USAGE_INFO \
	"\n\nUsage: crawler [<url> | <num_threads> <url_file.txt>]" "\n" \
	"- <num_threads> must always be = 1"						"\n" \
	"- <url> = scheme://host[:port][/path][?query][#fragment]""\n" \
	"\t-> scheme must always be \"http\""					"\n" \
	"\t-> port must be a valid 16-bit unsigned integer"

#endif //PCH_H
	