/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"

class url_exception : public std::exception
{
public: 
	explicit url_exception(const std::string& msg = "") : err("failed with " + msg) {}
	virtual const char* what() const throw() { return err.c_str(); }
private:
	std::string err;
};

class Url
{
public:
	static Url from(const std::string& url);
	static std::optional<Url> try_from(const std::string& url);

	std::string http_get() const;
	std::string http_read() const;

private:
	std::string host;
	std::string request;
	uint16_t port;

	Url(std::string&& host, std::string&& req, uint16_t port)
		: host{ host }, request{ req }, port{ port }
	{
		printf("host %s, port %d, request %s\n", host.c_str(), port, request.c_str());
	}

	inline std::string http(const std::string& method) const;

	static inline uint16_t parse_to_u16(const std::string& s);
	static inline std::optional<std::string> extract_after(std::string& str, char ch, int skip = 0);

	friend class std::optional<Url>;
	friend class std::shared_ptr<Url>;
	friend class Socket;
};

