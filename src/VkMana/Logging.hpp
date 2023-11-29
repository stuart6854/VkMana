#pragma once

#include <fmt/format.h>

#include <iostream>

#define VM_ERR(...)                                                      \
	do                                                                    \
	{                                                                     \
		std::cerr << "[ERROR] " << fmt::format(__VA_ARGS__) << std::endl; \
	}                                                                     \
	while (false)

#define VM_WARN(...)                                                    \
	do                                                                   \
	{                                                                    \
		std::cout << "[WARN] " << fmt::format(__VA_ARGS__) << std::endl; \
	}                                                                    \
	while (false)

#define VM_INFO(...)                                                    \
	do                                                                   \
	{                                                                    \
		std::cout << "[INFO] " << fmt::format(__VA_ARGS__) << std::endl; \
	}                                                                    \
	while (false)