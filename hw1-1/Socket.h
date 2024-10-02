/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"
#include "Url.h"
#include "HtmlResponse.h"

#pragma comment(lib, "ws2_32.lib")

class socket_exception : public std::exception
{
public:
	explicit socket_exception(const std::string& msg = "") : err("failed with " + msg) {}
	virtual const char* what() const throw() { return err.c_str(); }
private:
	std::string err;
};

class winsock_exception : public std::exception
{
public:
	explicit winsock_exception(const std::string& msg = "") : err("failed with " + std::to_string(h_errno) + " " + msg) {}
	virtual const char* what() const throw() { return err.c_str(); }
private:
	std::string err;
};


class Socket
{
	SOCKET sock;

public:
	Socket();
	~Socket();

	void request(const Url& url) const;


	void connect(const std::string& hostname, const uint16_t port) const;
	void disconnect() const;
	
	void send(const std::string& req) const;
	std::shared_ptr<HtmlResponse> response() const;

private:

	inline bool receive(std::vector<char>& buff, WSAEVENT listener) const;

	friend class std::shared_ptr<Socket>;
};

