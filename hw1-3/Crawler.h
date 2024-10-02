// Crawler.cpp : This file contains the state of the program, and useful action methods
/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"

#include "Url.h"
#include "Request.h"
#include "Tcp.h"
#include "WinSock.h"

#include "MMapFile.h"

#include "Mutex.h"
#include "Iter.h"

struct argument_error : public std::runtime_error {
	explicit argument_error(const std::string& msg = "") : std::runtime_error(msg + USAGE_INFO) {}
};

class Crawler
{
	struct Stats {
		std::atomic_uint16_t n_active_threads;
		std::atomic_uint32_t n_passed_dns;
		std::atomic_uint32_t n_passed_robots;
		std::atomic_uint32_t n_pages_crawled;
		std::atomic_uint32_t n_page_bytes;
		std::atomic_uint32_t n_bytes_downloaded;
		std::atomic_uint32_t n_total_links;
#ifdef GET_IN_TAMU
		std::atomic_uint32_t n_pages_w_tamu_internal;
		std::atomic_uint32_t n_pages_w_tamu_external;
#endif // GET_IN_TAMU

		std::atomic_uint32_t n_2xx;
		std::atomic_uint32_t n_3xx;
		std::atomic_uint32_t n_4xx;
		std::atomic_uint32_t n_5xx;
		std::atomic_uint32_t n_xxx;

		Stats()
			: n_active_threads(0), n_passed_dns(0), n_passed_robots(0)
			, n_pages_crawled(0), n_page_bytes(0), n_bytes_downloaded(0), n_total_links(0)
#ifdef GET_IN_TAMU
			, n_pages_w_tamu_internal(0)
			, n_pages_w_tamu_external(0)
#endif // GET_IN_TAMU
			, n_2xx(0), n_3xx(0), n_4xx(0), n_5xx(0), n_xxx(0) {}
	};

	WinSock w;
	std::chrono::high_resolution_clock::time_point start_time;

	// Sets for checking uniqueness
	sync::Mutex<std::set<uint32_t>> ips;
	sync::Mutex<std::set<std::string>> hosts;

	// The shared list of urls to crawl
	sync::Mutex<Iter<std::vector<std::string>>> urls;

	// The crawler threads
	std::vector<std::jthread> threads;

	// The stats thread
	std::jthread logger;

	Stats stats;

public:
	Crawler(const std::string& n_threads, const std::string& file_path)
		: Crawler(parse_threads(n_threads), MMapFile(file_path)) {}

	Crawler(const uint32_t n_threads, const MMapFile& file)
		: w()
		, start_time(Time::now())
		, ips()
		, hosts()
		, urls(file.begin(), file.end())
		, threads(n_threads)
		, logger()
		, stats()
	{
		std::cout.setstate(std::ios_base::failbit);
	}

	Crawler& lauch();
	Crawler& await();
	int summary() const;


	/**
	* Fetch the contents of the given url, counting the links and printing the header
	* 
	* If URL is invalid or malformed, print helpful usage and abort
	* If any socket errors occur, print the error and gracefully abort
	*/
	static int fetch(const std::string& url);

private:
	/**
	* Page Crawl
	*
	* Check if the host prevents robots,
	* if not, crawl the page at the given url, counting the links
	* If URL is invalid or malformed, print helpful usage and return
	* If fatal socket errors occur, print the error and gracefully abort, otherwise return
	*/
	int crawl(const std::string& url, std::unique_ptr<HTMLParserBase>& parser);

	std::tuple<uint32_t, uint32_t> log_stats(uint32_t prev_n_pages, uint32_t prev_n_bytes) const;

	static uint32_t parse_threads(const std::string& t) {
		uint32_t n_threads = UInt::parse<uint32_t>(t).value_or(-1);
		if (n_threads < 1) throw argument_error("invalid number of threads");
		return n_threads;
	}

	friend class Tcp;
};