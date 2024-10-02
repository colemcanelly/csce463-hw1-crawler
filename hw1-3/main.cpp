// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#include "pch.h"
#include <iostream>


#include "Crawler.h"


int main(int argc, char* argv[])
try {
	switch (argc)
	{
	case 1: throw argument_error("no arguments given");

	case 2: return Crawler::fetch(argv[1]);
	case 3:
		return Crawler(argv[1], argv[2])
			.lauch()
			.await()
			.summary();
	case 4:
	default: throw argument_error("too many arguments");
	}

	return 0;
}
catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
	return -1;
}