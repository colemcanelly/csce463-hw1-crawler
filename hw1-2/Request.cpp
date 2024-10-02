/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#include "pch.h"

#include "Request.h"

namespace Http {
	// //////////////////////////////////////////////////
	// Public Methods
	Request Request::from(const std::string& scheme_url, bool print_req) {
		printf("\tParsing URL... ");
		if (scheme_url.length() > MAX_URL_LEN) throw url_error("invalid url");
		if (!scheme_url.starts_with("http://")) throw url_error("invalid scheme");

		const size_t SCHEME_LEN = 7;
		std::string url = scheme_url.substr(SCHEME_LEN);

		extract_after(url, '#');
		std::string query = extract_after(url, '?').value_or("");

		std::string req = extract_after(url, '/').value_or("/") + query;
		std::string port_str = extract_after(url, ':', 1).value_or("80");
		std::string host(url.c_str());

		// Error checking exceeding for max length
		if (host.length() > MAX_HOST_LEN) throw url_error("invalid host (exceeded maximum length)");
		if (req.length() >= MAX_REQUEST_LEN) throw url_error("invalid request (exceeded maximum length)");

		auto port = UInt::parse<uint16_t>(port_str);
		if (!port) throw url_error("invalid port");

		if (!print_req) printf("host %s, port %hu\n", host.c_str(), port.value());
		else printf("host %s, port %hu, request %s\n", host.c_str(), port.value(), req.c_str());
		return Request(std::move(host), std::move(req), port.value());
	}

	Request& Request::check_unique(std::set<std::string>& hosts) {
		printf("\tChecking host uniqueness... ");
		if (!hosts.insert(this->host).second) throw uniqueness_error();
		printf("passed\n");
		return *this;
	}

	Request& Request::check_unique(std::set<uint32_t>& ips) {
		printf("\tChecking IP uniqueness... ");
		if (!ips.insert(this->ip).second) throw uniqueness_error();
		printf("passed\n");
		return *this;
	}

	// //////////////////////////////////////////////////+
	// Private Methods

	std::string Request::http(const std::string& req) const {
		std::string type;
		switch (this->method) {
		default:
		case Type::GET: type = "GET"; break;
		case Type::HEAD: type = "HEAD"; break;
		}
		return
			type + " " + req + " HTTP/1.0"	"\r\n" \
			"Host: " + this->host + "\r\n" \
			"User-agent: coleMcCrawler/1.2"	"\r\n" \
			"Connection: close"				"\r\n" \
			"\r\n";
	}


	// //////////////////////////////////////////////////
	// Static Helpers
	inline std::optional<std::string> Request::extract_after(std::string& str, char ch, int skip) {
		const char* ptr = strchr(str.c_str(), ch);
		if (ptr == nullptr) return {};
		std::string ret(ptr + skip);
		str[ptr - str.c_str()] = '\0';
		return ret;
	}
}