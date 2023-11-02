#pragma once

#include <cstdio>
#include <format>
#include <iostream>

#define LOG_ERR(...)                                                      \
	do                                                                    \
	{                                                                     \
		std::cerr << "[ERROR] " << std::format(__VA_ARGS__) << std::endl; \
	}                                                                     \
	while (false)

#define LOG_WARN(...)                                                    \
	do                                                                   \
	{                                                                    \
		std::cout << "[WARN] " << std::format(__VA_ARGS__) << std::endl; \
	}                                                                    \
	while (false)

#define LOG_INFO(...)                                                    \
	do                                                                   \
	{                                                                    \
		std::cout << "[INFO] " << std::format(__VA_ARGS__) << std::endl; \
	}                                                                    \
	while (false)