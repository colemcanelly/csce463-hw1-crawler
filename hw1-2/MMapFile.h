/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"

struct fatal_file_error : public std::runtime_error {
	explicit fatal_file_error(const std::string& msg = "") : std::runtime_error("failed to " + msg) {}
};


// Memory-mapped file reading for blazingly fast Read
class MMapFile
{
	// Handling the file
	HANDLE file_handle;
	HANDLE file_mapping;
	char* file;

	// Reading the file
	char* line_start;
	char* eof;

public:
	MMapFile(const std::string& file_path)
		: file_handle(nullptr)
		, file_mapping(nullptr)
		, file(nullptr)
		, line_start(nullptr)
		, eof(nullptr)
	{
		// Open the file
		file_handle = CreateFileA(file_path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file_handle == INVALID_HANDLE_VALUE) throw fatal_file_error("open file");

		// Create a file mapping object
		file_mapping = CreateFileMapping(file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (file_mapping == nullptr) throw fatal_file_error("create file mapping");

		// Map the file into memory
		file = (char*)MapViewOfFile(file_mapping, FILE_MAP_READ, 0, 0, 0);
		if (file == nullptr) throw fatal_file_error("map view of file");

		// Get the file size
		uint32_t size = GetFileSize(file_handle, nullptr);
		if (size == INVALID_FILE_SIZE) throw fatal_file_error("get file size");

		line_start = file;
		eof = file + size;

		printf("Opened %s with size %u\n", file_path.c_str(), size);
	}

	~MMapFile() {
		if (file) UnmapViewOfFile(file);
		if (file_mapping) CloseHandle(file_mapping);
		if (file_handle) CloseHandle(file_handle);
		file_handle = nullptr;
		file_mapping = nullptr;
		file = nullptr;
		line_start = nullptr;
		eof = nullptr;
	}

	// TODO: implement C++ iterator for more ergonomic reading into vector
	std::optional<std::string> getline() {
		if (line_start >= eof) return {};

		char* line_end = strchr(line_start, '\n');
		line_end = (line_end == nullptr) ? eof : line_end;
		std::string ret(line_start, line_end - 1);

		line_start = line_end + 1;
		return ret;
	}

};