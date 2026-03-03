#include "Logger.hpp"

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>


std::shared_ptr<spdlog::logger> g_logger = nullptr;

void Logger::init() {
    if (g_logger != nullptr) {
        LOG_WARN("Logger already initialized.");
        return;
    }

    g_logger = spdlog::stdout_color_mt("MAINR");

    g_logger->set_pattern("[%T] %^%n: %v%$");

#ifndef NDEBUG
    g_logger->set_level(spdlog::level::trace);
#else
    g_logger->set_level(spdlog::level::info);
#endif

    LOG_INFO("Logger initialized.");
}

spdlog::logger &Logger::getLogger() {
    assert(g_logger);
    return *g_logger;
}
