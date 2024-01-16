#pragma once

#include <fmt/format.h>

#include <iostream>

// #TODO: Allow user to set log/message callback

#define VM_INFO(...) std::cout << "[INFO] " << fmt::format(__VA_ARGS__) << std::endl
#define VM_WARN(...) std::cout << "[WARN] " << fmt::format(__VA_ARGS__) << std::endl
#define VM_ERR(...) std::cerr << "[ERROR] " << fmt::format(__VA_ARGS__) << std::endl
