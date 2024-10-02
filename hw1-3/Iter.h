/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/

#pragma once

#include "pch.h"

template <typename T>
class Iter
{
	T data;
	size_t index;

public:
	using value_type = T::value_type;

	Iter(T&& _data) : data(std::move(_data)), index(0) {}

	template<typename... Args>
	explicit Iter(Args&&... args) : data(std::forward<Args>(args)...), index(0) {}

	std::optional<value_type> next() {
		if (index >= data.size()) return {};
		return data.at(index++);
	}

	inline size_t remaining() const { return data.size() - index; }
	inline size_t position() const { return index; }
};
