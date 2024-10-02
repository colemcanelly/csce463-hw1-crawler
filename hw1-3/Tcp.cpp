/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#include "pch.h"
#include "Tcp.h"

#include "Request.h"
#include "Forwards.h"

#pragma comment(lib, "ws2_32.lib")

// //////////////////////////////////////////////////
// Public Methods

Tcp::Tcp(Crawler* _c, Download _d)
	: sock(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
	, max_download(PAGE_DOWNLOAD_LIMIT)
	, c(_c)
	, d(_d)
{
	if (sock == INVALID_SOCKET) throw fatal_winsock_error("couldn't create socket!");
}

Tcp::~Tcp() {
	closesocket(this->sock);
}

const Tcp& Tcp::request(const Http::Request& req, bool robots) {
	connect(req.ip, req.port, robots);
	if (robots) send(req.http_robots());
	else send(req.http());
	max_download = robots ? ROBOTS_DOWNLOAD_LIMIT : PAGE_DOWNLOAD_LIMIT;
	return *this;
}

void Tcp::connect(const uint32_t host_ip, const uint16_t port, bool robots) const {
	struct sockaddr_in server {};
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = host_ip;
	server.sin_port = htons(port);

	if (robots) std::cout << "\tConnecting on robots... ";
	else std::cout << "\t\b\b* Connecting on page... ";

	auto start = Time::now();

	int err = ::connect(this->sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
	if (err == SOCKET_ERROR) throw winsock_error("(Connection failure)");

	std::cout << "done in " << time_elapsed<milliseconds>(start) << "\n";
}

void Tcp::send(const std::string& req) const {
	int err = ::send(this->sock, req.c_str(), req.length(), NULL);
	if (err == SOCKET_ERROR) throw winsock_error();
}

std::shared_ptr<Http::Response> Tcp::response() const {
	std::cout << "\tLoading... ";
	auto start = Time::now();

	//WSAEVENT event_listener = WSACreateEvent();
	WSAEVENT event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (event == WSA_INVALID_EVENT) throw fatal_winsock_error("WSACreateEvent()");

	if (WSAEventSelect(this->sock, event, FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		WSACloseEvent(event);
		throw fatal_winsock_error("WSAEventSelect()");
	}

	// Read into buffer (auto resizing as necessary
	std::vector<char> buffer;
	buffer.reserve(THRESHOLD);
	auto time_started = Time::now();
	while (this->receive(buffer, event, time_started));
	if (buffer.empty()) throw socket_error("no response");
	buffer.push_back('\0');     // Null-terminate the buffer

	WSACloseEvent(event);

	if (strncmp(buffer.data(), "HTTP/", 5)) throw socket_error("non-HTTP header (does not begin with HTTP/)");

	std::cout << "done in " << time_elapsed<milliseconds>(start) << " with " << buffer.size() - 1 << " bytes\n";
	return std::make_shared<Http::Response>(buffer);
}


// //////////////////////////////////////////////////
// Private Methods

bool Tcp::receive(std::vector<char>& buf, WSAEVENT event, Time::time_point start) const {
	if ((d == Download::BOUNDED) && time_elapsed<milliseconds>(start) >= TIMEOUT) throw socket_error("slow download");
	uint32_t waitResult = WaitForSingleObject(event, TIMEOUT.count());

	if (waitResult == WSA_WAIT_TIMEOUT) throw socket_error("timeout");
	if (waitResult == WSA_WAIT_FAILED) throw fatal_winsock_error("WSAWaitForMultipleEvents() failed");

	WSANETWORKEVENTS events;
	if (WSAEnumNetworkEvents(sock, event, &events) == SOCKET_ERROR)
		throw fatal_winsock_error("WSAEnumNetworkEvents()");

	if (events.lNetworkEvents == NULL) return true;

	// Can read
	if (events.lNetworkEvents & FD_READ) {
		size_t position = buf.size();

		// Realloc if low on space
		if (buf.capacity() - position < THRESHOLD) buf.reserve(buf.capacity() * 2);

		buf.resize(buf.capacity());
		int num_bytes = ::recv(this->sock, buf.data() + position, buf.capacity() - position, 0);

		if (num_bytes < 0) throw winsock_error("in recv");	// Error in recv
		if (num_bytes == 0) return false;					// Connection closed by server

		buf.resize(position + num_bytes);
		if (c) c->stats.n_bytes_downloaded += num_bytes;
		if ((d == Download::BOUNDED) && buf.size() > max_download) throw socket_error("exceeding max");
	}

	// Connection closed
	if (events.lNetworkEvents & FD_CLOSE) return false;

	return true;
}
