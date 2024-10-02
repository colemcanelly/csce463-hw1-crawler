/*
 * Cole McAnelly
 * CSCE 463 - Dist. Network Systems
 * Fall 2024
 */

#pragma once

#include "pch.h"

struct parser_error : public std::runtime_error {
	explicit parser_error(const std::string& msg = "") : std::runtime_error("failed with " + msg) {}
};

struct dechunk_error : public std::runtime_error {
	explicit dechunk_error(const std::string& msg = "") : std::runtime_error("failed with " + msg) {}
};

constexpr size_t CRLF_LEN = 2;

namespace Http
{
	struct ResponseCode {
		enum class Type : uint8_t { _1xx = 1, _2xx, _3xx, _4xx, _5xx, };

		uint16_t code;
		Type type;

		ResponseCode(uint16_t _code) : code(_code), type(Type(_code / 100)) {}
		ResponseCode& operator = (uint16_t _code) {
			code = _code;
			type = Type(_code / 100);
			return *this;
		}
	};

	class Response
	{
		size_t length;
		bool chunked;
		ResponseCode status;
		std::string head;
		std::vector<char> body;

	public:
		// default: 418 "I'm a teapot" -> https://datatracker.ietf.org/doc/html/rfc2324#section-2.3.2
		Response(size_t len) : length(len), chunked(false), status(418), head(""), body{} {}

		Response(std::vector<char>& res);

		std::tuple<uint32_t, bool> parse(const std::string& url, const std::unique_ptr<HTMLParserBase>& parser) const;
		Response& dechunk();

		inline bool operator == (const ResponseCode::Type& rhs) const {
			std::cout << "\tVerifying header... status code " << status.code << "\n";
			return this->status.type == rhs;
		}

		inline std::string header() const {
			return "----------------------------------------\n" + head;
		}

		inline ResponseCode get_status() const { return status; }
		constexpr size_t size() const { return length; }
		constexpr bool is_chunked() const { return chunked; }

	private:
		static inline std::vector<char> extract_after(std::vector<char>& haystack, const std::string& needle);
	};

}
