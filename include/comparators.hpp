#pragma once

#include "book.hpp"

namespace bookdb::comp {

struct LessByAuthor {
    bool operator()(const Book &lhs, const Book &rhs) const {
        if (lhs.author == rhs.author)
            return lhs < rhs;
        return lhs.author < rhs.author;
    }
};

struct LessByTitle {
    bool operator()(const Book &lhs, const Book &rhs) const {
        if (lhs.title == rhs.title)
            return lhs < rhs;
        return lhs.title < rhs.title;
    }
};

struct LessByYear {
    bool operator()(const Book &lhs, const Book &rhs) const {
        if (lhs.year == rhs.year)
            return lhs < rhs;
        return lhs.year < rhs.year;
    }
};

struct LessByRating {
    bool operator()(const Book &lhs, const Book &rhs) const {
        if (std::abs(lhs.rating - rhs.rating) < 0.01)
            return lhs < rhs;
        return lhs.rating < rhs.rating;
    }
};

struct LessByPopularity {

    bool operator()(const Book &lhs, const Book &rhs) const {
        if (lhs.read_count == rhs.read_count)
            return lhs < rhs;
        return lhs.read_count < rhs.read_count;
    }
};

}  // namespace bookdb::comp