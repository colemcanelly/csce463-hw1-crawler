/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"

#include <concepts>

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
	MMapFile(const std::string& file_path);
	~MMapFile();

	std::optional<std::string> getline();

	class line_iterator {
		char* line_start;
		char* line_end;
		const char* eof;

	public:
		using value_type = std::string;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::input_iterator_tag;
		
		line_iterator(char* start, char* end);

		line_iterator& operator++ ();		// Prefix increment
		line_iterator operator++(int);		// Postfix increment
		inline std::string operator* () const { return std::string(line_start, line_end - 1); }
		inline bool operator!= (const line_iterator& other) const { return line_start != other.line_start; }
	};
	using iterator = line_iterator;

	iterator begin() const { return iterator(file, eof); }
	iterator end() const { return iterator(eof, eof); }		// sentinel
};