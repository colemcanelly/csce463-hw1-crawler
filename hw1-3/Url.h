/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once


#include "pch.h"


struct url_error : public std::runtime_error {
	explicit url_error(const std::string& msg = "") : std::runtime_error("failed with " + msg) {}
};


static inline std::optional<std::string> extract_after(std::string& str, char ch, int skip = 0) {
	const char* ptr = strchr(str.c_str(), ch);
	if (ptr == nullptr) return {};
	std::string ret(ptr + skip);
	str[ptr - str.c_str()] = '\0';
	return ret;
}

struct Url
{
	std::string host;
	std::string request;
	uint16_t port;
	bool in_tamu;

	Url(const std::string& scheme_url, bool silent = false)
		: host(""), request(""), port(0)
	{
		if (!silent) std::cout << "\tParsing URL... ";
		if (scheme_url.length() > MAX_URL_LEN) throw url_error("invalid url");
		if (!scheme_url.starts_with("http://")) throw url_error("invalid scheme");

		const size_t SCHEME_LEN = 7;
		std::string url = scheme_url.substr(SCHEME_LEN);

		extract_after(url, '#');
		std::string query = extract_after(url, '?').value_or("");

		request = extract_after(url, '/').value_or("/") + query;
		std::string port_str = extract_after(url, ':', 1).value_or("80");
		host = std::string(url.c_str());
		in_tamu = host.ends_with("tamu.edu");

		// Error checking exceeding for max length
		if (host.length() > MAX_HOST_LEN) throw url_error("invalid host (exceeded maximum length)");
		if (request.length() >= MAX_REQUEST_LEN) throw url_error("invalid request (exceeded maximum length)");

		port = UInt::parse<uint16_t>(port_str).value_or(0);
		if (!port) throw url_error("invalid port");

		if (!silent) std::cout << "host " << host << ", port " << port << ", request " << request << "\n";
	}
};