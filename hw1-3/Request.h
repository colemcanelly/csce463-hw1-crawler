/*
 * Cole McAnelly
 * CSCE 463 - Dist. Network Systems
 * Fall 2024
 */

#pragma once

#include "pch.h"
#include "Forwards.h"
#include "WinSock.h"
#include "Mutex.h"
#include "Url.h"

struct uniqueness_error : public std::runtime_error {
	explicit uniqueness_error() : std::runtime_error("failed") {}
};

namespace Http
{
	class Request
	{
	public:

		static Request from(const std::string &url);
		Request(const Url& _u) : Request(_u.host, _u.request, _u.port) {}

		// Request builder
		inline Request& get() {
			method = Type::GET;
			return *this;
		}

		// Request builder
		inline Request& head() {
			method = Type::HEAD;
			return *this;
		}

		inline Request& check_dns() {
			ip = WinSock::dns(host);
			return *this;
		}

		inline Request& check_dns(std::atomic_uint32_t& count) {
			ip = WinSock::dns(host);
			count++;
			return *this;
		}
		
		Request& check_unique(sync::Mutex<std::set<std::string>>& hosts);
		Request& check_unique(sync::Mutex<std::set<uint32_t>>& ips);

		// Generate an HTTP request string with the given method
		inline std::string http() const { return this->http(this->request); }
		inline std::string http_robots() const { return this->http("/robots.txt"); }

	private:
		Request(const std::string& host, const std::string& req, uint16_t port)
			: host{host}, request{req}, ip(0), port(port), method(Type::GET)
		{}

		std::string http(const std::string& req) const;

		enum class Type { GET, HEAD };


		std::string host;
		std::string request;
		uint32_t ip;
		uint16_t port;
		Type method;

		friend class std::shared_ptr<Request>;
		friend class Tcp;
		friend class Crawler;
	};
}
