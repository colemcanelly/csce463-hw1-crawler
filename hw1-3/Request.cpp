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
	Request Request::from(const std::string& scheme_url) {
		return Request(Url(scheme_url));
	}

	Request& Request::check_unique(sync::Mutex<std::set<std::string>>& hosts) {
		std::cout << "\tChecking host uniqueness... ";
		if (!hosts.lock()->insert(this->host).second) throw uniqueness_error();
		std::cout << "passed\n";
		return *this;
	}

	Request& Request::check_unique(sync::Mutex<std::set<uint32_t>>& ips) {
		std::cout << "\tChecking IP uniqueness... ";
		if (!ips.lock()->insert(this->ip).second) throw uniqueness_error();
		std::cout << "passed\n";
		return *this;
	}

	// //////////////////////////////////////////////////+
	// Private Methods
#ifdef HTTP1_1
#define HTTP_VERSION	" HTTP/1.1"
#else  
#define HTTP_VERSION	" HTTP/1.0"
#endif // HTTP1_1
	std::string Request::http(const std::string& req) const {
		std::string type;
		switch (this->method) {
		default:
		case Type::GET: type = "GET"; break;
		case Type::HEAD: type = "HEAD"; break;
		}
		return
			type + " " + req + HTTP_VERSION	"\r\n" \
			"Host: " + this->host + "\r\n" \
			"User-agent: coleMcCrawler/1.3"	"\r\n" \
			"Connection: close"				"\r\n" \
			"\r\n";

	}
}