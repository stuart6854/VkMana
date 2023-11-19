#pragma once

#include <cstdio>
#include <format>
#include <iostream>

#define VM_ERR(...)                                                      \
	do                                                                    \
	{                                                                     \
		std::cerr << "[ERROR] " << std::format(__VA_ARGS__) << std::endl; \
	}                                                                     \
	while (false)

#define VM_WARN(...)                                                    \
	do                                                                   \
	{                                                                    \
		std::cout << "[WARN] " << std::format(__VA_ARGS__) << std::endl; \
	}                                                                    \
	while (false)

#define VM_INFO(...)                                                    \
	do                                                                   \
	{                                                                    \
		std::cout << "[INFO] " << std::format(__VA_ARGS__) << std::endl; \
	}                                                                    \
	while (false)