/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"
#pragma comment(lib, "ws2_32.lib")

// RAII for winsock DLL
class WinSock {
public:
	WinSock()
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			int err = WSAGetLastError();
			WSACleanup();
			throw std::runtime_error("WSAStartup failed with " + std::to_string(err));
		}
	}

	~WinSock() { WSACleanup(); }
};
