#pragma once

#include <functional>

namespace vkm {

	// From: https://stackoverflow.com/a/57595105 using C++17 fold expressions
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

}