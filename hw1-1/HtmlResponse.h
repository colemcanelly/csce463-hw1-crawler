/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"


class HtmlResponse
{
	uint16_t status_code;
	std::string status;
	std::string head;
	std::string body;

public:
	HtmlResponse() : status_code(), status(""), head(""), body("") {}

	HtmlResponse(const std::string& _res) : HtmlResponse() {
		std::string res(_res);
		body = extract_after(res, "\r\n\r\n").value_or("");
		head = std::string(res.c_str());

		extract_after(res, "\r\n");
		status = std::string(res.c_str());
		status_code = static_cast<uint16_t>(std::stoul(status.substr(9, 3)));
	}

	void parse(const std::string& url) const {
		printf("\t\b\b+ Parsing page... ");
		auto start = TIME_CURRENT;

		int link_count = 0;

		HTMLParserBase parser;
		parser.Parse((char*)body.c_str(), body.length(), (char*)url.c_str(), url.length(), &link_count);


		std::cout << "done in " << TIME_ELAPSED(start) << " with " << link_count << " links\n";
	}

	bool verify_header() const {
		printf("\tVerifying header... status code %hu\n", this->status_code);
		return 200 <= status_code && status_code < 300;
	}

	std::string header() const {
		return "----------------------------------------\n" + head;
	}

private:
	static inline std::optional<std::string> extract_after(std::string& haystack, const std::string& needle, int skip = 0) {
		const char* ptr = strstr(haystack.c_str(), needle.c_str());
		if (ptr == nullptr) return {};
		std::string ret(ptr + skip);
		haystack[ptr - haystack.c_str()] = '\0';
		return ret;
	}
};
