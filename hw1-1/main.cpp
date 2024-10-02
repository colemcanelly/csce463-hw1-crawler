// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#include "pch.h"
#include <iostream>

#include "Url.h"
#include "Socket.h"
#include "WinSock.h"

int main(int argc, char* argv[]) {
	try {
		if (argc < 2) throw url_exception("no URL provided");
		if (argc > 2) throw url_exception("too many arguments");
		
		// Initialize winsock api
		WinSock init;

		// Print requested URL
		printf("URL: %s\n", argv[1]);

		// Transform URL to requestable format
		std::string base_url(argv[1]);
		Url req = Url::from(base_url);
		
		// Send request, and wait for response
		Socket sock;
		sock.request(req);
		auto res = sock.response();

		// Check for 2xx code, parse if present, then print the header
		if (res->verify_header()) res->parse(base_url);
		std::cout << res->header() << std::endl;
	}
	catch (const url_exception& url_err) {
		std::cout << url_err.what()
			<< "\n\nExample Usage: \n"
			<< "./crawler  <url>\n\n"
			<< "<url> = scheme://host[:port][/path][?query][#fragment]\n"
			<< "\t-> scheme must be `http`\n"
			<< "\t-> port must be a valid 16-bit unsigned integer" << std::endl;
		return -1;

	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		return -1;
	}
	return 0;
}