#pragma once

#include <chrono>
#include <format>
#include <stdexcept>
#include <string_view>

namespace bookdb {

// обязательно Unknown последний
enum class Genre : size_t { Fiction, NonFiction, SciFi, Biography, Mystery, Unknown };

// Концепт для Genre
template <typename T>
concept GenreClass = std::is_same_v<std::remove_cvref_t<T>, Genre> ||
                     (std::convertible_to<T, std::string_view> && !std::is_same_v<std::remove_cvref_t<T>, std::string>);

template <typename T>
concept IsGenreOrStringRef = std::is_same_v<std::remove_cvref_t<T>, Genre> || std::is_same_v<T, const std::string &>;

constexpr std::string StringFromGenre(Genre genre) {

    switch (genre) {
    case Genre::Fiction:
        return "Fiction";
    case Genre::Mystery:
        return "Mystery";
    case Genre::NonFiction:
        return "NonFiction";
    case Genre::SciFi:
        return "SciFi";
    case Genre::Biography:
        return "Biography";
    default:
        return "Unknown";
    }
}

constexpr Genre GenreFromString(std::string_view s) {

    if (s == "Fiction")
        return Genre::Fiction;

    if (s == "Mystery")
        return Genre::Mystery;

    if (s == "NonFiction")
        return Genre::NonFiction;

    if (s == "SciFi")
        return Genre::SciFi;

    if (s == "Biography")
        return Genre::Biography;

    return Genre::Unknown;
}

template <GenreClass Genre_1, GenreClass Genre_2>
constexpr bool GenreEqual(Genre_1 lhs, Genre_2 rhs) {

    if constexpr (std::is_convertible_v<Genre_1, Genre_2>) {
        return lhs == rhs;
    } else {

        if constexpr (std::is_same_v<Genre, Genre_1>) {
            return lhs == GenreFromString(rhs);
        } else {
            return GenreFromString(lhs) == rhs;
        }
    }
}

constexpr Genre GenreCast(Genre g) { return g; }
constexpr Genre GenreCast(std::string_view str) { return GenreFromString(str); }

template <typename T>
void hash_combine(size_t &seed, const T &val) {
    std::hash<T> hasher;
    seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

constexpr int GetCurrentYear() {
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    std::tm *local_time = std::localtime(&current_time);

    return local_time->tm_year + 1900;
}

struct Book {
    // string_view для экономии памяти, чтобы ссылаться на оригинальную строку, хранящуюся в другом контейнере
    std::string_view author;  // для замены указателей
    std::string title;

    int year;
    Genre genre;
    double rating;
    int read_count;

    // Дружественная функция swap
    friend void swap(Book &first, Book &second) noexcept {
        using std::swap;

        swap(first.author, second.author);
        swap(first.title, second.title);
        swap(first.year, second.year);
        swap(first.genre, second.genre);
        swap(first.rating, second.rating);
        swap(first.read_count, second.read_count);
    }

    void swap(Book &other) noexcept {
        using std::swap;
        swap(*this, other);
    }

    template <GenreClass C>
    constexpr Book(const std::string &title_, std::string_view author_, int year_, C genre_, double rating_,
                   int read_count_)
        : author(author_), title(title_), year(ValidateYear(year_)), genre(GenreCast(genre_)),
          rating(ValidateRating(rating_)), read_count(ValidateReadCount(read_count_)) {}

    size_t GetHash() const {
        size_t seed = 0;

        hash_combine(seed, author);
        hash_combine(seed, title);
        hash_combine(seed, year);
        hash_combine(seed, static_cast<int>(genre));

        return seed;
    }

    auto operator<=>(const Book &other) const = default;

private:
    constexpr int ValidateYear(int year) const {

        if (year < 1400 || year > GetCurrentYear())
            throw std::invalid_argument("Publication year has to be bigger than 1400 and less than current year");

        return year;
    }

    constexpr double ValidateRating(double rating) const {

        if (rating < 0.0 || rating > 5.0)
            throw std::invalid_argument("Rating must to be a number between 0 and 5");

        return rating;
    }

    constexpr int ValidateReadCount(int read_count) const {

        if (read_count < 0)
            throw std::invalid_argument("The number of readers must be non-negative");

        return read_count;
    }
};

template <GenreClass C>
size_t GetBookHash(std::string_view title, std::string_view author, int year, C genre) {
    size_t seed = 0;

    hash_combine(seed, author);
    hash_combine(seed, title);
    hash_combine(seed, year);
    hash_combine(seed, static_cast<int>(GenreCast(genre)));

    return seed;
}

}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::Genre, char> {
    template <typename FormatContext>
    auto format(const bookdb::Genre g, FormatContext &fc) const {
        std::string genre_str;

        // clang-format off
        using bookdb::Genre;
        switch (g) {
            case Genre::Fiction:    genre_str = "Fiction"; break;
            case Genre::Mystery:    genre_str = "Mystery"; break;
            case Genre::NonFiction: genre_str = "NonFiction"; break;
            case Genre::SciFi:      genre_str = "SciFi"; break;
            case Genre::Biography:  genre_str = "Biography"; break;
            case Genre::Unknown:    genre_str = "Unknown"; break;
            default:
                throw logic_error{"Unsupported bookdb::Genre"};
            }
        // clang-format on
        return format_to(fc.out(), "{}", genre_str);
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

template <>
struct formatter<bookdb::Book, char> {
    template <typename FormatContext>
    auto format(const bookdb::Book &book, FormatContext &fc) const {
        return format_to(fc.out(),
                         "\"{}\" written by {}. Publication year is {}. Genre: {}. Rating: {}. Read count: {}",
                         book.title, std::string(book.author), book.year, book.genre, book.rating, book.read_count);
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

}  // namespace std
