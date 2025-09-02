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

struct TransparentStringHash {

    using is_transparent = void;

    template <StringLike T, StringLike U>
    bool operator()(const T &lhs, const U &rhs) const {
        return std::hash<T>{}(lhs) == std::hash<U>(rhs);
    }
};

}  // namespace bookdb
