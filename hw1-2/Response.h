/*
 * Cole McAnelly
 * CSCE 463 - Dist. Network Systems
 * Fall 2024
 */

#pragma once

#include "pch.h"

namespace Http
{
	enum class StatusCode {
		_1xx = 1,
		_2xx,
		_3xx,
		_4xx,
		_5xx,
	};

	class Response
	{
		uint16_t status_code;
		std::string head;
		std::vector<char> body;

	public:
		Response() : status_code(), head(""), body{} {}

		Response(std::vector<char> &res) : Response()
		{
			body = extract_after(res, "\r\n\r\n");
			head = std::string(res.data());

			status_code = UInt::parse<uint16_t>(head.substr(9, 3)).value();
		}

		void parse(const std::string& url) const
		{
			printf("\t\b\b+ Parsing page... ");
			auto start = TIME_CURRENT;

			int link_count = 0;

			HTMLParserBase parser;
			parser.Parse((char*)body.data(), body.size(), (char*)url.c_str(), url.length(), &link_count);

			std::cout << "done in " << TIME_ELAPSED(start) << " with " << link_count << " links\n";
		}

		bool operator== (const StatusCode& rhs) const {
			printf("\tVerifying header... status code %hu\n", this->status_code);
			return (this->status_code / 100) == static_cast<int>(rhs);
		}

		std::string header() const
		{
			return "----------------------------------------\n" + head;
		}

	private:
		static inline std::vector<char> extract_after(std::vector<char>& haystack, const std::string &needle)
		{
			const char* ptr = strstr(haystack.data(), needle.c_str());
			if (ptr == nullptr)
				return {};
			const char* start = ptr + needle.length();
			const char* end = haystack.data() + haystack.size();
			std::vector<char> ret(start, end);
			haystack[ptr - haystack.data()] = '\0';
			return ret;
		}
	};

}
