#pragma once

#include <algorithm>
#include <functional>

#include "book.hpp"
#include "concepts.hpp"

namespace bookdb {

// Фабрика предикатов для года издания
auto YearBetween(int start, int end) -> BookPredicate auto {
    return [start, end](const Book &book) { return book.year >= start && book.year <= end; };
}

// Фабрика предикатов для рейтинга
auto RatingAbove(double min_rating) -> BookPredicate auto {
    return [min_rating](const Book &book) { return book.rating > min_rating; };
}

// Фабрика предикатов для жанра
template <GenreClass genre_type>
auto GenreIs(const genre_type &genre) -> BookPredicate auto {
    return [genre](const Book &book) { return GenreEqual(book.genre, genre); };
}

// Комбинатор ALL - все предикаты должны быть истинны
template <BookPredicate... Predicates>
auto all_of(Predicates... predicates) -> BookPredicate auto {
    return [=](const Book &book) { return (predicates(book) && ...); };
}

// Комбинатор ANY - хотя бы один предикат должен быть истинным
template <BookPredicate... Predicates>
auto any_of(Predicates... predicates) -> BookPredicate auto {
    return [=](const Book &book) { return (predicates(book) || ...); };
}

template <BookIterator It, BookPredicate Predicate>
std::vector<std::reference_wrapper<const Book>> filterBooks(It first, It second, Predicate predicate) {

    std::vector<std::reference_wrapper<const Book>> result;
    std::copy_if(first, second, std::back_inserter(result), predicate);

    return result;
}

}  // namespace bookdb