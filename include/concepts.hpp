#pragma once

#include <concepts>
#include <iterator>

#include "book.hpp"

namespace bookdb {

template <typename Container>
concept TemplateBookContainer = requires(Container container) {
    requires requires {
        container.begin();
        container.end();
        container.size();
        container.empty();
    };
    { std::sort(container.begin(), container.end()) } -> std::same_as<void>;
};

template <typename T>
concept BookContainerLike =
    requires(T container) { requires std::same_as<typename T::value_type, Book> && TemplateBookContainer<T>; };

template <typename T>
concept BookReferenceContainerLike = requires(T container) {
    requires std::same_as<typename T::value_type, std::reference_wrapper<Book>> ||
                 std::same_as<typename T::value_type, std::reference_wrapper<const Book>>;
    requires requires(typename T::value_type element) {
        {
            element.get()
        } -> std::same_as<std::add_lvalue_reference_t<std::conditional_t<
            std::same_as<typename T::value_type, std::reference_wrapper<const Book>>, const Book, Book>>>;
    };
    requires TemplateBookContainer<T>;
};

template <typename T>
concept BookIterator = std::input_iterator<T> && std::same_as<std::iter_value_t<T>, Book>;

template <typename S, typename I>
concept BookSentinel =
    std::sentinel_for<S, I> && requires(I iter) { requires std::same_as<std::iter_value_t<I>, Book>; };

template <typename P>
concept BookPredicate = requires(P p, const Book &book) {
    { p(book) } -> std::same_as<bool>;
};

template <typename C>
concept BookComparator = std::invocable<C, Book, Book> && std::same_as<std::invoke_result_t<C, Book, Book>, bool>;

}  // namespace bookdb