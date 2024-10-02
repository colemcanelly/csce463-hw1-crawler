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
#include <sys/types.h>
#include <string>
#include <iostream>
#include <memory>
#include <optional>
#include <limits>
#include <exception>
#include <chrono>

#include "HTMLParserBase.h"

#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>


#define TIME_CURRENT std::chrono::high_resolution_clock::now()
#define TIME_ELAPSED(begin) std::chrono::duration_cast<std::chrono::milliseconds>(TIME_CURRENT - begin)

#endif //PCH_H
