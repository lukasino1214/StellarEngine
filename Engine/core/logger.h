#pragma once

#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Engine {
    class Logger {
    public:
        static void init();

        static std::shared_ptr<spdlog::logger> core_logger;
        static std::shared_ptr<spdlog::logger> client_logger;
    };

}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream &operator<<(OStream &os, const glm::vec<L, T, Q> &vector) {
    return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream &operator<<(OStream &os, const glm::mat<C, R, T, Q> &matrix) {
    return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream &operator<<(OStream &os, glm::qua<T, Q> quaternion) {
    return os << glm::to_string(quaternion);
}

// Core log macros
#define CORE_TRACE(...)    ::Engine::Logger::core_logger->trace(__VA_ARGS__)
#define CORE_INFO(...)     ::Engine::Logger::core_logger->info(__VA_ARGS__)
#define CORE_WARN(...)     ::Engine::Logger::core_logger->warn(__VA_ARGS__)
#define CORE_ERROR(...)    ::Engine::Logger::core_logger->error(__VA_ARGS__)
#define CORE_CRITICAL(...) ::Engine::Logger::core_logger->critical(__VA_ARGS__)

// Client log macros
#define TRACE(...)         ::Engine::Logger::client_logger->trace(__VA_ARGS__)
#define INFO(...)          ::Engine::Logger::client_logger->info(__VA_ARGS__)
#define WARN(...)          ::Engine::Logger::client_logger->warn(__VA_ARGS__)
#define ERROR(...)         ::Engine::Logger::client_logger->error(__VA_ARGS__)
#define CRITICAL(...)      ::Engine::Logger::client_logger->critical(__VA_ARGS__)