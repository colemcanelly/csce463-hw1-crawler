/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"
#pragma comment(lib, "ws2_32.lib")

// Error hierarchy for fatal and recoverable errors (fatal errors are higher, and therefore won't be caught until later
/* Exception hierarchy
fatal_socket error
|-- fatal_winsock error
|   '-- winsock error
'-- socket error
*/

struct fatal_socket_error : public std::runtime_error {
	explicit fatal_socket_error(const std::string& msg = "") : std::runtime_error("failed with " + msg) {}
};

struct fatal_winsock_error : public fatal_socket_error {
	explicit fatal_winsock_error(const std::string& msg = "") : fatal_socket_error(std::to_string(h_errno) + " " + msg) {}
};

struct socket_error : public fatal_socket_error {
	explicit socket_error(const std::string& msg = "") : fatal_socket_error(msg) {}
};

struct winsock_error : public fatal_winsock_error {
	explicit winsock_error(const std::string& msg = "") : fatal_winsock_error(msg) {}
};


// RAII for winsock DLL
class WinSock {
public:
	WinSock()
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) throw fatal_winsock_error("WSAStartup");
	}

	~WinSock() { WSACleanup(); }

	static inline uint32_t dns(const std::string& hostname) {
		// Got info from reading inaddr.h
		printf("\tDoing DNS... ");
		auto start = TIME_CURRENT;
		in_addr ip;
		ip.s_addr = inet_addr(hostname.c_str());
		if (ip.s_addr == INADDR_NONE) {
			hostent* host = gethostbyname(hostname.c_str());
			if (host == NULL) throw winsock_error("(DNS failure)");;
			ip.s_addr = *reinterpret_cast<uint32_t*>(host->h_addr);
		}
		std::cout << "done in " << TIME_ELAPSED(start) << ", found " << inet_ntoa(ip) << "\n";
		return ip.s_addr;
	}
};
