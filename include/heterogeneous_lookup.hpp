#pragma once

#include <string>
#include <string_view>

namespace bookdb {

template <typename T>
concept StringLike = requires(const T &t) { requires std::convertible_to<const T &, std::string_view>; };

struct TransparentStringLess {
    using is_transparent = void;

    template <StringLike T, StringLike U>
    bool operator()(const T &lhs, const U &rhs) const {
        return lhs < rhs;
    }
};

struct TransparentStringEqual {
    using is_transparent = void;

    template <StringLike T, StringLike U>
    bool operator()(const T &lhs, const U &rhs) const {
        return lhs == rhs;
    }
};

struct StringHash {
    using is_transparent = void;

    // Оператор для вычисления хэша
    template <StringLike T>
    size_t operator()(const T &key) const {
        return std::hash<std::string_view>{}(std::string_view(key));
    }

    // Операторы сравнения для прозрачности (опционально)
    template <StringLike T, StringLike U>
    bool operator()(const T &lhs, const U &rhs) const {
        return lhs == rhs;
    }
};

struct TransparentStringHash {

    using is_transparent = void;

    template <StringLike T, StringLike U>
    bool operator()(const T &lhs, const U &rhs) const {
        return std::hash<T>{}(lhs) == std::hash<U>(rhs);
    }
};

}  // namespace bookdb
