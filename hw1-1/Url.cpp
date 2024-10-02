/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#include "pch.h"

#include "Url.h"

// //////////////////////////////////////////////////
// Public Methods


std::string Url::http_get() const { return this->http("GET"); }
std::string Url::http_read() const { return this->http("READ"); }

Url Url::from(const std::string& scheme_url) {
	printf("\tParsing URL... ");
	if (scheme_url.length() > MAX_URL_LEN) throw url_exception("invalid url");
	if (!scheme_url.starts_with("http://")) throw url_exception("invalid scheme");

	const size_t SCHEME_LEN = 7;
	std::string url = scheme_url.substr(SCHEME_LEN);

	extract_after(url, '#');
	std::string query = extract_after(url, '?').value_or("");

	std::string req = extract_after(url, '/').value_or("/") + query;
	std::string port = extract_after(url, ':', 1).value_or("80");
	std::string host(url.c_str());

	// Error checking exceeding for max length
	if (host.length() > MAX_HOST_LEN) throw url_exception("invalid host (exceeded maximum length)");
	if (req.length() >= MAX_REQUEST_LEN) throw url_exception("invalid request (exceeded maximum length)");

	//return std::make_unique<Url>(std::move(host), path + query, port.value());
	return Url(std::move(host), std::move(req), parse_to_u16(port));
}

// for testing
std::optional<Url> Url::try_from(const std::string& url) {
	try {
		return Url::from(url);
	}
	catch (const url_exception& e) {
		std::cout << e.what() << std::endl;
		return {};
	}
}



// //////////////////////////////////////////////////
// Private Methods

// Generate an HTTP request string with the given method
inline std::string Url::http(const std::string& method) const {
	return
		method + " " + request + " HTTP/1.0  \r\n" \
		"Host: " + host +					"\r\n" \
		"User-agent: coleMcCrawler/1.1"     "\r\n" \
		"Connection: close"                 "\r\n" \
		"\r\n";
}


// //////////////////////////////////////////////////
// Static Helpers
inline std::optional<std::string> Url::extract_after(std::string& str, char ch, int skip) {
	const char* ptr = strchr(str.c_str(), ch);
	if (ptr == nullptr) return {};
	std::string ret(ptr + skip);
	str[ptr - str.c_str()] = '\0';
	return ret;
}



inline uint16_t Url::parse_to_u16(const std::string& s) {
	try {
		unsigned long port = std::stoul(s);
		if (port > std::numeric_limits<uint16_t>::max() || port == 0) throw url_exception("invalid port");
		return static_cast<uint16_t>(port);
	}
	catch (...) {
		throw url_exception("invalid port");
	}
}