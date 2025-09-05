#pragma once

#include <algorithm>
#include <flat_map>
#include <functional>
#include <iterator>
#include <numeric>
#include <random>

#include "book.hpp"
#include "book_database.hpp"
#include "comparators.hpp"
#include "concepts.hpp"

#include <print>

namespace bookdb {

template <BookContainerLike T, typename Comparator = TransparentStringLess>
auto buildAuthorHistogramFlat(const BookDatabase<T> &cont, Comparator comp = {}) {

    return std::flat_map<std::string, size_t, Comparator>(cont.GetAuthors().begin(), cont.GetAuthors().end(), comp);
};

// исключительно ради того, чтобы правильно работал std::format
struct GenreRating {

public:
    using Container = std::array<double, static_cast<size_t>(Genre::Unknown) + 1>;
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;
    using reverse_iterator = Container::reverse_iterator;
    using const_reverse_iterator = Container::const_reverse_iterator;

    iterator begin() noexcept { return rating.begin(); }
    iterator end() noexcept { return rating.end(); }

    const_iterator begin() const noexcept { return rating.begin(); }
    const_iterator end() const noexcept { return rating.end(); }
    const_iterator cbegin() const noexcept { return rating.cbegin(); }
    const_iterator cend() const noexcept { return rating.cend(); }

    // Реверсные итераторы
    reverse_iterator rbegin() noexcept { return rating.rbegin(); }
    reverse_iterator rend() noexcept { return rating.rend(); }

    const_reverse_iterator rbegin() const noexcept { return rating.rbegin(); }
    const_reverse_iterator rend() const noexcept { return rating.rend(); }
    const_reverse_iterator crbegin() const noexcept { return rating.crbegin(); }
    const_reverse_iterator crend() const noexcept { return rating.crend(); }

    bool empty() const { return rating.empty(); }
    size_t size() const { return rating.size(); }
    Container &data() { return rating; }
    const Container &data() const { return rating; }

    ////

    template <GenreClass Genre_>
    double get(Genre_ genre) const {
        // ошибка для случая, когда неверно выделили память (кто-то добавил жанр после Unknown)
        if (size_t size = static_cast<size_t>(GenreCast(genre)); size >= rating.size()) {
            return 0.0;
        } else {
            return rating[static_cast<size_t>(GenreCast(genre))];
        }
    }

private:
    Container rating;
};

template <BookIterator Iter, BookSentinel<Iter> Sent>
[[nodiscard]] GenreRating calculateGenreRatings(Iter begin, Sent end) {

    std::array<std::pair<double, size_t>, static_cast<size_t>(Genre::Unknown) + 1> out_buf;

    std::for_each(begin, end, [&out_buf](const Book &book) {
        out_buf[static_cast<size_t>(book.genre)].first += book.rating;
        ++out_buf[static_cast<size_t>(book.genre)].second;
    });

    GenreRating result;

    std::transform(out_buf.begin(), out_buf.end(), result.begin(), [](const std::pair<double, size_t> &vec) {
        if (vec.second == 0)
            return 0.0;
        return vec.first / vec.second;
    });

    return result;
}

template <BookContainerLike T>
[[nodiscard]] double calculateAverageRating(const BookDatabase<T> &cont) {

    auto &books = cont.GetBooks();
    if (books.empty())
        return 0;

    double sum = std::transform_reduce(books.begin(), books.end(), 0.0, std::plus<>(),
                                       [](const Book &product) { return product.rating; });

    return sum / books.size();
}

template <BookContainerLike T>
[[nodiscard]] std::vector<std::reference_wrapper<const Book>> sampleRandomBooks(const BookDatabase<T> &cont,
                                                                                size_t number_out) {

    auto &books = cont.GetBooks();

    if (number_out >= books.size()) {
        return {books.begin(), books.end()};
    }

    std::vector<std::reference_wrapper<const Book>> result;
    std::sample(cont.begin(), cont.end(), std::back_inserter(result), number_out, std::mt19937{std::random_device{}()});

    return result;
}

template <BookContainerLike T, BookComparator Comp = comp::LessByRating>
[[nodiscard]] std::vector<std::reference_wrapper<const Book>> getTopNBy(BookDatabase<T> &cont, size_t number_out,
                                                                        Comp comp = {}) {

    if (number_out <= 0 || cont.empty())
        return {};
    if (number_out >= cont.size()) {
        std::sort(cont.begin(), cont.end(), comp);
        return {cont.begin(), cont.end()};
    }

    // Частичная сортировка - находим n-й наибольший элемент
    std::nth_element(cont.begin(), cont.begin() + number_out, cont.end(), comp);

    // Сортируем только первые n элементов
    std::sort(cont.begin(), cont.begin() + number_out, comp);

    return std::vector<std::reference_wrapper<const Book>>(cont.begin(), cont.begin() + number_out);
}

}  // namespace bookdb

namespace std {

template <typename Comparator>
struct formatter<std::flat_map<std::string, size_t, Comparator>, char> {

    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const std::flat_map<std::string, size_t, Comparator> &hist, FormatContext &ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{");

        for (const auto &[author, count] : hist) {
            out = std::format_to(out, "\n\t\"{}\": {};", author, count);
        }

        out = std::format_to(out, "\n}}");
        return out;
    }
};

template <>
struct formatter<bookdb::GenreRating, char> {

    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const bookdb::GenreRating &hist, FormatContext &ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{");

        auto &rating = hist.data();

        for (size_t i = 0; i < rating.size(); ++i) {
            out = std::format_to(out, "\n\t\"{}\": {:.2f};", static_cast<bookdb::Genre>(i), rating[i]);
        }

        out = std::format_to(out, "\n}}");
        return out;
    }
};

}  // namespace std
