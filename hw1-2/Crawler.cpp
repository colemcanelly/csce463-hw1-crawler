/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#include "pch.h"

#include "Crawler.h"
#include "Request.h"


int Crawler::fetch(const std::string& url) const
try {
	// Print requested URL
	printf("URL: %s\n", url.c_str());

	// Transform URL to requestable format
	Http::Request req = Http::Request::from(url)
		.do_dns()
		.get();

	// Send request, and wait for response
	auto res = Tcp().request(req).response(false);

	// Check for 2xx code, parse if present, then print the header
	if (*res == Http::StatusCode::_2xx) res->parse(url);
	std::cout << res->header() << std::endl;
	return 0;
}
catch (const url_error& url_err) {
	throw argument_error(url_err.what());
}



int Crawler::run(const std::string& threads, const std::string& file_path) {
	auto port = UInt::parse<uint32_t>(threads);
	if (port.value_or(-1) != 1) throw argument_error("invalid number of threads");

	std::vector<std::string> urls;
	MMapFile file(file_path);
	while (auto url = file.getline()) urls.push_back(url.value());

	for (const std::string& url : urls) crawl(url);

	return 0;
}

void Crawler::crawl(const std::string& url)
try {
	// Print requested URL
	printf("URL: %s\n", url.c_str());

	// Transform URL to requestable format
	Http::Request req = Http::Request::from(url, false)
		.check_unique(hosts)
		.do_dns()
		.check_unique(ips);

	// Send request, and wait for response
	auto res = Tcp()
		.request(req.head(), true)
		.response();

	// Ensure that `robots.txt` does not exist
	if (*res != Http::StatusCode::_4xx) return;

	// Send request, and wait for response
	res = Tcp().request(req.get()).response();

	// Check for 2xx code, parse if present, then print the header
	if (*res == Http::StatusCode::_2xx) res->parse(url);
}
catch (const url_error& e) {
	printf("%s\n", e.what());
}
catch (const uniqueness_error& e) {
	printf("%s\n", e.what());
}
catch (const socket_error& e) {
	printf("%s\n", e.what());
}
catch (const winsock_error& e) {
	printf("%s\n", e.what());
}