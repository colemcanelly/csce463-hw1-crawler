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
#include "Crawler.h"

#pragma comment(lib, "ws2_32.lib")

constexpr milliseconds TIMEOUT = 10s; // 10 seconds in milliseconds
constexpr size_t THRESHOLD = 4KiB;

constexpr size_t PAGE_DOWNLOAD_LIMIT = 2MB;
constexpr size_t ROBOTS_DOWNLOAD_LIMIT = 16KB;

class Tcp
{
public:
	enum class Download: uint8_t { BOUNDED, UNBOUNDED };
private:
	SOCKET sock;
	size_t max_download;
	Crawler* c;
	Download d;

public:
	Tcp(Crawler* _c, Download _d = Download::BOUNDED);
	~Tcp();

	const Tcp& request(const Http::Request& req, bool robots = false);

	std::shared_ptr<Http::Response> response() const;

	void connect(const uint32_t host_ip, const uint16_t port, bool robots) const;
	void send(const std::string& req) const;

private:

	bool receive(std::vector<char>& buff, WSAEVENT listener, Time::time_point start) const;

	friend class std::shared_ptr<Tcp>;
};

