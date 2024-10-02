/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#include "pch.h"

#include "Crawler.h"
#include "Request.h"

// //////////////////////////////////////////////////
// Public Methods

Crawler& Crawler::lauch() {
	// Start stats threads
	logger = std::jthread(
		[this](std::stop_token st) {
			uint32_t n_pages = 0, n_bytes = 0;
			while (!st.stop_requested()) {
				std::this_thread::sleep_for(2s);
				std::tie(n_pages, n_bytes) = log_stats(n_pages, n_bytes);
			}
		}
	);

	// Start worker threads
	for (auto&& t : threads) t = std::jthread(
		[this]() {
			stats.n_active_threads++;
			auto parser = std::make_unique<HTMLParserBase>();

			// Main loop
			while (auto url = urls.lock()->next())
				stats.n_total_links += crawl(url.value(), parser);

			stats.n_active_threads--;
		}
	);

	return *this;
}

Crawler& Crawler::await() {
	// Wait for workers to finish
	for (auto& t : threads) t.join();

	// Signal stats thread to stop, clean up
	logger.request_stop();
	logger.join();
	return *this;
}

int Crawler::summary() const {
	uint16_t elapsed = time_elapsed<seconds>(start_time).count();
	uint32_t extracted = urls.lock()->position();
	uint32_t unique_hosts = hosts.lock()->size();
	uint32_t unique_ips = ips.lock()->size();
	uint32_t n_pages = stats.n_pages_crawled.load();
	uint32_t n_page_bytes = stats.n_page_bytes.load();
	uint32_t n_links = stats.n_total_links.load();

	printf("\n");
	printf("Extracted %u URLs @ %u/s\n", extracted, extracted / elapsed);
	printf("Looked up %u DNS names @ %u / s\n", unique_hosts, unique_hosts / elapsed);
	printf("Attempted %u site robots @ %u / s\n", unique_ips, unique_ips / elapsed);
	printf("Crawled %u pages @ %u / s (%.2f MB)\n", n_pages, n_pages / elapsed, unit_cast<std::mega>(double(n_page_bytes)));
	printf("Parsed %u links @ %u / s\n", n_links, n_links / elapsed);
	printf("HTTP codes : 2xx = %u, 3xx = %u, 4xx = %u, 5xx = %u, other = %u\n",
		stats.n_2xx.load(), stats.n_3xx.load(), stats.n_4xx.load(), stats.n_5xx.load(), stats.n_xxx.load()
	);

#ifdef GET_IN_TAMU
	printf("\n");
	printf("Internal TAMU links: %u\n", stats.n_pages_w_tamu_internal.load());
	printf("External TAMU links: %u\n", stats.n_pages_w_tamu_external.load());
#endif // GET_IN_TAMU

	return 0;
}


// //////////////////////////////////////////////////
// Private Methods

int Crawler::crawl(const std::string& url, std::unique_ptr<HTMLParserBase>& parser)
try {
	// Print requested URL
	std::cout << "URL: " << url << "\n";

	// Transform URL to requestable format
	Url parsed_url(url);
	Http::Request req = Http::Request(parsed_url)
		.check_unique(hosts)
		.check_dns(stats.n_passed_dns)
		.check_unique(ips);

	// Send request, and wait for response
	auto res = Tcp(this)
		.request(req.head(), true)
		.response();

	// Ensure that `robots.txt` does not exist
	if (*res != Http::ResponseCode::Type::_4xx) return 0;
	stats.n_passed_robots++;

	// Send request, and wait for response
	res = Tcp(this)
		.request(req.get())
		.response();

	stats.n_pages_crawled++;
	stats.n_page_bytes += res->size();

	// Check for 2xx code, parse if present, then print the header
	switch (res->get_status().type)
	{
	default:
	case Http::ResponseCode::Type::_1xx: stats.n_xxx++; return 0;
	case Http::ResponseCode::Type::_2xx: {
		stats.n_2xx++;
#ifdef HTTP1_1
		if (res->is_chunked()) res->dechunk();
#endif // HTTP1_1
		auto [n_links, contains_tamu_links] = res->parse(url, parser);
#ifdef GET_IN_TAMU
		if (contains_tamu_links) {
			if (parsed_url.in_tamu) stats.n_pages_w_tamu_internal++;
			else stats.n_pages_w_tamu_external++;
		}
#endif // GET_IN_TAMU
		return n_links;
	}
	case Http::ResponseCode::Type::_3xx: stats.n_3xx++; return 0;
	case Http::ResponseCode::Type::_4xx: stats.n_4xx++; return 0;
	case Http::ResponseCode::Type::_5xx: stats.n_5xx++; return 0;
	}
}
catch (const url_error& e) {
	std::cout << e.what() << "\n";
	return 0;
}
catch (const uniqueness_error& e) {
	std::cout << e.what() << "\n";
	return 0;
}
catch (const socket_error& e) {
	std::cout << e.what() << "\n";
	return 0;
}
catch (const dechunk_error& e) {
	std::cout << e.what() << "\n";
	return 0;
}
catch (const parser_error& e) {
	std::cout << e.what() << "\n";
	return 0;
}
catch (const winsock_error& e) {
	std::cout << e.what() << "\n";
	return 0;
}

std::tuple<uint32_t, uint32_t> Crawler::log_stats(uint32_t prev_n_pages, uint32_t prev_n_bytes) const
{
	uint16_t elapsed = time_elapsed<seconds>(start_time).count();
	uint32_t queue = urls.lock()->remaining();
	uint32_t extracted = urls.lock()->position();
	uint32_t unique_hosts = hosts.lock()->size();
	uint32_t unique_ips = ips.lock()->size();

	uint32_t n_pages = stats.n_pages_crawled.load();
	uint32_t n_bytes = stats.n_bytes_downloaded.load();
	printf(
		"[%3hu] %4hu Q %6u E %7u H %6u D %6u I %5u R %5u C %5u L %4dK\n"
		"\t\b\b*** crawling %.1f pps @ %.1f Mbps\n",
		elapsed,
		stats.n_active_threads.load(),
		queue,	/* Q */
		extracted,	/* E */
		unique_hosts,		/* H */
		stats.n_passed_dns.load(),			/* D */
		unique_ips,		/* I */
		stats.n_passed_robots.load(),		/* R */
		n_pages,							/* C */
		unit_cast<std::kilo>(stats.n_total_links.load()),			/* L */
		double(n_pages - prev_n_pages) / elapsed,
		unit_cast<std::mega, bytes, double>(n_bytes - prev_n_bytes) / elapsed
	);
	return std::make_tuple(n_pages, n_bytes);
}



// //////////////////////////////////////////////////
// Public Static methods (hw1p1)

int Crawler::fetch(const std::string& url)
try {
	WinSock ws_init;

	// Print requested URL
	std::cout << "URL: " << url << "\n";

	// Transform URL to requestable format
	Http::Request req = Http::Request::from(url)
		.check_dns()
		.get();

	// Send request, and wait for response
	auto res = Tcp(nullptr, Tcp::Download::UNBOUNDED)
		.request(req)
		.response();

	// Check for 2xx code, parse if present, then print the header
	if (*res == Http::ResponseCode::Type::_2xx) {
#ifdef HTTP1_1
		if (res->is_chunked()) res->dechunk();
#endif // HTTP1_1
		res->parse(url, std::make_unique<HTMLParserBase>());
	}
	std::cout << res->header() << std::endl;
	return 0;
}
catch (const url_error& url_err) {
	throw argument_error(url_err.what());
}