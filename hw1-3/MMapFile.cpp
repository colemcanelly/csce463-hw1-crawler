/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"

#include "MMapFile.h"

// Static helper function
inline static char* next_line(char* start, const char* eof) {
	if (start == eof) return (char*)eof;
	char* line_end = strchr(start, '\n');
	return (line_end == nullptr) ? (char*)eof : line_end;
}

// Default constructor
MMapFile::MMapFile(const std::string& file_path)
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

	std::cout << "Opened " << file_path << " with size " << size << "\n";
}

MMapFile::~MMapFile() {
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
std::optional<std::string> MMapFile::getline() {
	if (line_start >= eof) return {};

	char* line_end = next_line(line_start, eof);
	std::string ret(line_start, line_end - 1);

	line_start = line_end + 1;
	return ret;
}

MMapFile::line_iterator::line_iterator(char* start, char* end)
	: line_start(start)
	, line_end(next_line(start, end))
	, eof(end)
{}

MMapFile::line_iterator& MMapFile::line_iterator::operator++() {
	line_start = (line_end == eof ? (char*)eof : line_end + 1);
	line_end = next_line(line_start, eof);
	return *this;
}

MMapFile::line_iterator MMapFile::line_iterator::operator++(int) {
	line_iterator tmp = *this;
	++*this;
	return tmp;
}
