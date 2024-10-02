/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#include "pch.h"

#include "Response.h"
#include "Url.h"

#include <shlwapi.h>

static inline char* strcasestr(const char* haystack, const char* needle) { return StrStrIA(haystack, needle); }

namespace Http {
	// //////////////////////////////////////////////////
	// Public Methods
	Response::Response(std::vector<char>& res) : Response(res.size())
	{
		body = extract_after(res, "\r\n\r\n");
		head = std::string(res.data());

		status = UInt::parse<uint16_t>(head.substr(9, 3)).value();
#ifdef HTTP1_1
		chunked = strcasestr(head.c_str(), "Transfer-Encoding: chunked") != NULL;
#endif // HTTP1_1
	}

	Response& Response::dechunk() {
		std::cout << "\tDechunking... ";
		size_t size_before = body.size();
		if (size_before == 0) return *this;

		char* dst = body.data();
		char* src = body.data();

		while (src < body.data() + size_before) {
			char* size_end = strstr(src, "\r\n");
			if (!size_end) break;
			*size_end = 0;			// Null terminate the chunk size
			
			auto maybe_chunk_size = UInt::parse<size_t>(std::string(src, size_end - src), 16);
			if (!maybe_chunk_size) throw dechunk_error("invalid chunk size");

			src = size_end + CRLF_LEN;
			size_t chunk_size = maybe_chunk_size.value();
			if (chunk_size == 0) break;

			if (src + chunk_size > body.data() + size_before) chunk_size = body.data() + size_before - src;
			memcpy(dst, src, chunk_size);
			src += chunk_size + CRLF_LEN;
			dst += chunk_size;
		}
		*dst = 0;
		body.resize(++dst - body.data());

		std::cout << "body size was " << size_before - 1 << ", now " << body.size() - 1 << "\n";
		return *this;
	}

	std::tuple<uint32_t, bool> Response::parse(const std::string& url, const std::unique_ptr<HTMLParserBase>& parser) const
	{
		std::cout << "\t\b\b+ Parsing page... ";
		auto start = Time::now();

		int n_links = 0;
		bool contains_tamu_links = false;

		char* links = parser->Parse((char*)body.data(), body.size(), (char*)url.c_str(), url.length(), &n_links);
		if (n_links < 0) throw parser_error("internal parser error - " + std::to_string(n_links));
		
#ifdef GET_IN_TAMU
		for (int _ = 0; _ < n_links; _++, links += strlen(links) + 1)
			if (Url(links, true).in_tamu) 
				contains_tamu_links = true;
#endif // GET_IN_TAMU


		std::cout << "done in " << time_elapsed<milliseconds>(start) << " with " << n_links << " links\n";
		return std::make_tuple((uint32_t)n_links, contains_tamu_links);
	}

	// //////////////////////////////////////////////////
	// Static Helpers
	inline std::vector<char> Response::extract_after(std::vector<char>& haystack, const std::string& needle)
	{
		const char* ptr = strstr(haystack.data(), needle.c_str());
		if (ptr == nullptr)	return {};
		const char* start = ptr + needle.length();
		const char* end = haystack.data() + haystack.size();
		std::vector<char> ret(start, end);
		haystack[ptr - haystack.data()] = '\0';
		return ret;
	}
}