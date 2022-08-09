#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace Engine {
    template <typename T> using Ref = std::shared_ptr<T>;
    template <typename T, typename... Args>
    constexpr Ref<T> create_ref(Args &&...args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    struct ResultErr {
        std::string message = {};
    };

    template <typename T> struct Result {
        std::optional<T> v = {};
        std::string m = {};

    public:
        Result(T &&value) : v{std::move(value)}, m{""} {}
        Result(T const &value) : v{value}, m{""} {}
        Result(std::optional<T> &&opt) : v{std::move(opt)}, m{opt.has_value() ? "" : "default error message"} {}
        Result(std::optional<T> const &opt) : v{opt}, m{opt.has_value() ? "" : "default error message"} {}
        Result(ResultErr const &err) : v{std::nullopt}, m{err.message} {}
        Result(std::string_view message) : v{std::nullopt}, m{message} {}

        bool is_ok() const { return v.has_value(); }
        bool is_err() const { return !v.has_value(); }

        T const& value() const {
            return v.value();
        }

        T & value() {
            if(!v.has_value()) {
                throw std::runtime_error((!m.empty() ? m : "tried getting value of empty Result"));
            }
            return v.value();
        }

        const std::string& message() const { return m; }
        operator bool() const { return v.has_value(); }
        bool operator!() const { return !v.has_value(); }
    };
} // namespace Engine