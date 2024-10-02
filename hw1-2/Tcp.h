/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"

#include "WinSock.h"
#include "Request.h"
#include "Response.h"

#pragma comment(lib, "ws2_32.lib")

constexpr uint32_t TIMEOUT = 10000; // 10 seconds in milliseconds
constexpr size_t THRESHOLD = 4KiB;

constexpr size_t PAGE_DOWNLOAD_LIMIT = 2MB;
constexpr size_t ROBOTS_DOWNLOAD_LIMIT = 16KB;

class Tcp
{
	SOCKET sock;
	size_t max_download;

public:
	Tcp();
	~Tcp();

	const Tcp& request(const Http::Request& req, bool robots = false);

	std::shared_ptr<Http::Response> response(bool check_limits = true) const;

	void connect(const uint32_t host_ip, const uint16_t port, bool robots) const;
	void send(const std::string& req) const;

private:

	using time_point = std::chrono::steady_clock::time_point;
	bool receive(std::vector<char>& buff, WSAEVENT listener, time_point timeout, bool check_limits) const;

	friend class std::shared_ptr<Tcp>;
};

