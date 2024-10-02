/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#include "pch.h"
#include "Socket.h"

#include "Url.h"

// //////////////////////////////////////////////////
// Static Helpers

static uint32_t dns(const std::string& hostname) {
	// Got info from reading inaddr.h
	printf("\tDoing DNS... ");
	auto start = TIME_CURRENT;
	in_addr ip;
	ip.s_addr = inet_addr(hostname.c_str());
	if (ip.s_addr == INADDR_NONE) {
		hostent* host = gethostbyname(hostname.c_str());
		if (host == NULL) return INADDR_NONE;
		ip.s_addr = *reinterpret_cast<uint32_t*>(host->h_addr);
	}
	std::cout << "done in " << TIME_ELAPSED(start) << ", found " << inet_ntoa(ip) << "\n";
	return ip.s_addr;
}



// //////////////////////////////////////////////////
// Public Methods

Socket::Socket() : sock(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
{
	if (sock == INVALID_SOCKET) throw winsock_exception("couldn't create socket!");
}

Socket::~Socket() {
	closesocket(this->sock);
}



void Socket::request(const Url& url) const {
	connect(url.host, url.port);
	send(url.http_get());
}

void Socket::connect(const std::string& hostname, const uint16_t port) const {
	struct sockaddr_in server {};
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = dns(hostname);
	server.sin_port = htons(port);
	
	if (server.sin_addr.s_addr == INADDR_NONE) throw winsock_exception("(DNS failure)");

	printf("\t\b\b* Connecting on page... ");
	auto start = TIME_CURRENT;
	
	int err = ::connect(this->sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
	if (err == SOCKET_ERROR) throw winsock_exception("(Connection failure)");
	
	std::cout << "done in " << TIME_ELAPSED(start) << "\n";
}

void Socket::send(const std::string& req) const {
	int err = ::send(this->sock, req.c_str(), req.length(), NULL);
	if (err == SOCKET_ERROR) throw winsock_exception();
}

std::shared_ptr<HtmlResponse> Socket::response() const {
    printf("\tLoading... ");
    auto start = TIME_CURRENT;

    WSAEVENT event_listener = WSACreateEvent();
    if (event_listener == WSA_INVALID_EVENT) throw winsock_exception("WSACreateEvent()");

    if (WSAEventSelect(this->sock, event_listener, FD_READ | FD_CLOSE) == SOCKET_ERROR) {
        WSACloseEvent(event_listener);
        throw winsock_exception("WSAEventSelect()");
    }

    // Read into buffer (auto resizing as necessary
    std::vector<char> buffer;
    while (this->receive(buffer, event_listener));

    WSACloseEvent(event_listener);
    disconnect();

    if (buffer.empty()) throw socket_exception("no response");
    buffer.push_back('\0');     // Null-terminate the buffer

    std::string response(buffer.data(), buffer.size());
    if (!response.starts_with("HTTP/")) throw socket_exception("non-HTTP header (does not begin with HTTP/)");
    
    std::cout << "done in " << TIME_ELAPSED(start) << " with " << buffer.size() << " bytes\n";
    return std::make_shared<HtmlResponse>(response);
}

void Socket::disconnect() const {
    if (shutdown(this->sock, SD_SEND) == SOCKET_ERROR) {
        closesocket(this->sock);
        throw winsock_exception("disconnect failed");
    }
}


// //////////////////////////////////////////////////
// Private Methods

inline bool Socket::receive(std::vector<char>& buf, WSAEVENT listener) const {
    constexpr uint32_t TIMEOUT = 10000; // 10 seconds in milliseconds
    constexpr size_t CHUNK_SIZE = 4096;
    constexpr size_t THRESHOLD = 1024;

    uint32_t waitResult = WSAWaitForMultipleEvents(1, &listener, FALSE, TIMEOUT, FALSE);

    if (waitResult == WSA_WAIT_TIMEOUT) throw socket_exception("timeout");

    // Handle error
    if (waitResult == WSA_WAIT_FAILED) throw winsock_exception("WSAWaitForMultipleEvents() failed");

    WSANETWORKEVENTS events;
    if (WSAEnumNetworkEvents(sock, listener, &events) == SOCKET_ERROR)
        throw winsock_exception("WSAEventSelect()");

    if (events.lNetworkEvents & FD_READ) {
        size_t position = buf.size();

        // Realloc if low on space
        if (buf.capacity() - position < THRESHOLD) buf.reserve(buf.capacity() + CHUNK_SIZE);

        buf.resize(buf.capacity());
        int num_bytes = ::recv(this->sock, buf.data() + position, buf.capacity() - position, 0);

        if (num_bytes < 0) throw winsock_exception("in recv");   // Error in recv
        if (num_bytes == 0) return false;                       // Connection closed by server

        buf.resize(position + num_bytes);
    }

    // Connection closed
    if (events.lNetworkEvents & FD_CLOSE) return false;

    return true;
}