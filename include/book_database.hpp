#pragma once

#include <print>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "book.hpp"
#include "concepts.hpp"
#include "heterogeneous_lookup.hpp"

namespace bookdb {

template <BookContainerLike BookContainer = std::vector<Book>>
class BookDatabase {
public:
    // Type aliases

    using iterator = BookContainer::iterator;
    using const_iterator = BookContainer::const_iterator;
    using reverse_iterator = BookContainer::reverse_iterator;
    using const_reverse_iterator = BookContainer::const_reverse_iterator;

    using AuthorContainer = std::unordered_map<std::string, size_t>;

    BookDatabase() = default;

    BookDatabase(std::initializer_list<Book> init_list) {
        Reserve(init_list.size());

        for (const Book &book : init_list) {

            size_t hash = book.GetHash();

            // проверка на то, что такой книги еще нет
            if (auto it = books_unique_.find(hash); it != books_unique_.end())
                return;

            books_unique_.insert(hash);

            // Прямая обработка для эффективности
            std::string author_str(book.author);

            // Находим или создаем автора
            auto [it, inserted] = authors_.try_emplace(std::move(author_str), 1);
            if (!inserted) {
                it->second++;
            }

            // Создаем копию книги с правильным string_view
            Book new_book = book;
            new_book.author = std::string_view(it->first);
            books_.push_back(std::move(new_book));
        }
    }

    void Clear() {
        books_.clear();
        authors_.clear();
    }

    // Standard container interface methods

    // Итераторы доступа
    iterator begin() noexcept { return books_.begin(); }
    iterator end() noexcept { return books_.end(); }

    const_iterator begin() const noexcept { return books_.begin(); }
    const_iterator end() const noexcept { return books_.end(); }
    const_iterator cbegin() const noexcept { return books_.cbegin(); }
    const_iterator cend() const noexcept { return books_.cend(); }

    // Реверсные итераторы
    reverse_iterator rbegin() noexcept { return books_.rbegin(); }
    reverse_iterator rend() noexcept { return books_.rend(); }

    const_reverse_iterator rbegin() const noexcept { return books_.rbegin(); }
    const_reverse_iterator rend() const noexcept { return books_.rend(); }
    const_reverse_iterator crbegin() const noexcept { return books_.crbegin(); }
    const_reverse_iterator crend() const noexcept { return books_.crend(); }

    bool empty() const { return books_.empty(); }
    size_t size() const { return books_.size(); }

    ///////////////////////////////////////

    void PushBack(const Book &book) {

        size_t hash = book.GetHash();

        // проверка на то, что такой книги еще нет
        if (auto it = books_unique_.find(hash); it != books_unique_.end())
            return;

        books_unique_.insert(hash);

        std::string author_str(book.author);

        // Находим или создаем запись об авторе
        auto [it, inserted] = authors_.try_emplace(author_str, 1);

        if (!inserted) {
            // Автор уже существует, обновляем счетчик
            it->second++;
        }

        // Создаем копию книги с правильным string_view
        Book new_book = book;
        new_book.author = std::string_view(it->first);
        books_.push_back(std::move(new_book));
    }

    void PushBack(Book &&book) {

        size_t hash = book.GetHash();

        // проверка на то, что такой книги еще нет
        if (auto it = books_unique_.find(hash); it != books_unique_.end())
            return;

        books_unique_.insert(hash);

        std::string author_str(book.author);

        // Находим или создаем запись об авторе
        auto [it, inserted] = authors_.try_emplace(author_str, 1);

        if (!inserted) {
            // Автор уже существует, обновляем счетчик
            it->second++;
        }

        // Обновляем string_view и перемещаем книгу
        book.author = std::string_view(it->first);
        books_.push_back(std::move(book));
    }

    template <typename Title, typename Author, GenreClass Genre, typename... Args>
    void EmplaceBack(Title &&title, Author &&author, int year, Genre genre, Args &&...args) {

        size_t hash = GetBookHash(title, author, year, genre);

        // проверка на то, что такой книги еще нет
        if (auto it = books_unique_.find(hash); it != books_unique_.end())
            return;

        books_unique_.insert(hash);

        // Преобразуем автора в строку
        std::string author_str(std::forward<Author>(author));

        // Находим или создаем запись об авторе
        auto [it, inserted] = authors_.try_emplace(author_str, 1);

        if (!inserted) {
            // Автор уже существует, обновляем счетчик
            it->second++;
        }

        // Создаем книгу
        books_.emplace_back(std::forward<Title>(title), std::string_view(it->first), year, std::forward<Genre>(genre),
                            std::forward<Args>(args)...);
    }

    const BookContainer &GetBooks() const { return books_; }

    const AuthorContainer &GetAuthors() const { return authors_; }

    const std::vector<std::string_view> GetAuthorsList() const {
        std::vector<std::string_view> author_names;
        author_names.reserve(authors_.size());

        std::transform(authors_.begin(), authors_.end(), std::back_inserter(author_names),
                       [](const auto &author_pair) { return std::string_view(author_pair.first); });

        return author_names;
    }

private:
    void Reserve(size_t capacity) {
        if constexpr (requires {
                          books_.reserve(capacity);
                          authors_.reserve(capacity);
                      }) {
            books_.reserve(capacity);
            authors_.reserve(capacity);
        }
    }

private:
    BookContainer books_;
    std::unordered_set<size_t> books_unique_;
    AuthorContainer authors_;
};

}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::BookDatabase<std::vector<bookdb::Book>>> {
    template <typename FormatContext>
    auto format(const bookdb::BookDatabase<std::vector<bookdb::Book>> &db, FormatContext &fc) const {

        format_to(fc.out(), "BookDatabase (size = {}): ", db.size());

        format_to(fc.out(), "Books:\n");
        for (const auto &book : db.GetBooks()) {
            format_to(fc.out(), "- {}\n", book);
        }

        format_to(fc.out(), "Authors:\n");
        for (const auto &author : db.GetAuthorsList()) {
            format_to(fc.out(), "- {}\n", author);
        }

        return fc.out();
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

template <bookdb::BookContainerLike BookContainer>
struct formatter<BookContainer, char> {

    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const BookContainer &container, FormatContext &ctx) const {

        auto out = ctx.out();
        out = std::format_to(out, "{{");

        for (const auto &element : container) {
            out = std::format_to(out, "\n\t{}", element);
        }

        out = std::format_to(out, "\n}}");
        return out;
    }
};

template <bookdb::BookReferenceContainerLike BookContainer>
struct formatter<BookContainer, char> {

    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const BookContainer &container, FormatContext &ctx) const {

        auto out = ctx.out();
        out = std::format_to(out, "{{");

        for (const auto &element : container) {
            out = std::format_to(out, "\n\t{}", element.get());
        }

        out = std::format_to(out, "\n}}");
        return out;
    }
};

}  // namespace std
