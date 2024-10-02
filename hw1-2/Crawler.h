// Crawler.cpp : This file contains the state of the program, and useful action methods
/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"

#include "Request.h"
#include "Tcp.h"
#include "WinSock.h"
#include "MMapFile.h"


struct argument_error : public std::runtime_error {
	explicit argument_error(const std::string& msg = "") : std::runtime_error(msg + USAGE_INFO) {}
};

class Crawler
{
	WinSock w;
	std::set<uint32_t> ips;
	std::set<std::string> hosts;

public:
	Crawler() : w(WinSock()), ips{}, hosts{} {}

	int run(const std::string& threads, const std::string& file_path);

	/**
	* Fetch the contents of the given url, counting the links and printing the header
	* 
	* If URL is invalid or malformed, print helpful usage and abort
	* If any socket errors occur, print the error and gracefully abort
	*/
	int fetch(const std::string& url) const;

private:
	/**
	* Page Crawl
	*
	* Check if the host prevents robots,
	* if not, crawl the page at the given url, counting the links
	* If URL is invalid or malformed, print helpful usage and return
	* If fatal socket errors occur, print the error and gracefully abort, otherwise return
	*/
	void crawl(const std::string& url);
};