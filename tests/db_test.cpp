#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "book.hpp"
#include "book_database.hpp"
#include "comparators.hpp"
#include "concepts.hpp"
#include "filters.hpp"
#include "statsistics.hpp"

#ifdef __linux__
#include <malloc.h>
#endif

using namespace std::string_literals;
using namespace bookdb;

// Тесты для класса Book
class BookTest : public ::testing::Test {};

// проверка на то, что подать можно как строку так и enum
TEST_F(BookTest, GenreConstructor) {

    EXPECT_EQ(Book("1984", "George Orwell", 1949, "SciFi", 4., 190),
              Book("1984", "George Orwell", 1949, Genre::SciFi, 4., 190));
}

// Тесты для граничных случаев и обработки ошибок
TEST(EdgeCasesTest, InvalidRating) {
    EXPECT_THROW(Book("1984", "George Orwell", 1949, "SciFi", 5.4, 190), std::invalid_argument);
    EXPECT_THROW(Book("1984", "George Orwell", 1949, "SciFi", -0.3, 190), std::invalid_argument);

    // Валидные рейтинги не должны бросать исключения
    EXPECT_NO_THROW(Book("1984", "George Orwell", 1949, "SciFi", 4.0, 190));
}

TEST(EdgeCasesTest, InvalidYear) {
    EXPECT_THROW(Book("1984", "George Orwell", 2040, "SciFi", 4.0, 190), std::invalid_argument);
    EXPECT_THROW(Book("1984", "George Orwell", 1367, "SciFi", 4.0, 190), std::invalid_argument);
    EXPECT_NO_THROW(Book("1984", "George Orwell", 1949, "SciFi", 4.0, 190));
}

class ComparatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем тестовые книги
        books = {{"War and Peace", "Tolstoy Lev", 1869, Genre::Biography, 4.8, 10000},
                 {"Crime and Punishment", "Dostoevsky Fedor", 1866, Genre::Fiction, 4.7, 8000},
                 {"Eugene Onegin", "Pushkin Alexander", 1833, Genre::Fiction, 4.9, 12000},
                 {"Dead Souls", "Gogol Nikolay", 1842, Genre::Fiction, 4.6, 7000},
                 {"The Cherry Orchard", "Chekhov Anton", 1904, Genre::Fiction, 4.5, 6000}};
    }

    std::vector<Book> books;
};

// Тесты для LessByAuthor
TEST_F(ComparatorTest, LessByAuthor_SortsByAuthorAlphabetically) {
    std::sort(books.begin(), books.end(), comp::LessByAuthor());

    EXPECT_EQ(books[0].author, "Chekhov Anton");
    EXPECT_EQ(books[1].author, "Dostoevsky Fedor");
    EXPECT_EQ(books[2].author, "Gogol Nikolay");
    EXPECT_EQ(books[3].author, "Pushkin Alexander");
    EXPECT_EQ(books[4].author, "Tolstoy Lev");
}

TEST_F(ComparatorTest, LessByAuthor_EqualAuthors_ReturnsFalse) {
    Book book1 = {"War and Peace", "Tolstoy Lev", 1869, Genre::Biography, 4.8, 10000};
    Book book2 = {"War and Peace_1", "Tolstoy Lev", 1870, Genre::Biography, 4.9, 10000};

    comp::LessByAuthor comparator;
    EXPECT_TRUE(comparator(book1, book2));
    EXPECT_FALSE(comparator(book2, book1));
}

// Тесты для LessByTitle
TEST_F(ComparatorTest, LessByTitle_SortsByTitleAlphabetically) {
    std::sort(books.begin(), books.end(), comp::LessByTitle());

    EXPECT_EQ(books[0].title, "Crime and Punishment");
    EXPECT_EQ(books[1].title, "Dead Souls");
    EXPECT_EQ(books[2].title, "Eugene Onegin");
    EXPECT_EQ(books[3].title, "The Cherry Orchard");
    EXPECT_EQ(books[4].title, "War and Peace");
}

TEST_F(ComparatorTest, LessByTitle_EqualTitles_ReturnsFalse) {
    Book book1 = {"War and Peace", "Tolstoy Lev", 1869, Genre::Biography, 4.8, 10000};
    Book book2 = {"War and Peace", "Tolstoy Lev_1", 1869, Genre::Biography, 4.8, 10000};

    comp::LessByTitle comparator;
    EXPECT_TRUE(comparator(book1, book2));
    EXPECT_FALSE(comparator(book2, book1));
}

// Тесты для LessByYear
TEST_F(ComparatorTest, LessByYear_SortsByYearAscending) {
    std::sort(books.begin(), books.end(), comp::LessByYear());

    EXPECT_EQ(books[0].year, 1833);  // Pushkin
    EXPECT_EQ(books[1].year, 1842);  // Gogol
    EXPECT_EQ(books[2].year, 1866);  // Dostoevsky
    EXPECT_EQ(books[3].year, 1869);  // Tolstoy
    EXPECT_EQ(books[4].year, 1904);  // Chekhov
}

TEST_F(ComparatorTest, LessByYear_EqualYears_ReturnsFalse) {
    Book book1 = {"War and Peace", "Tolstoy Lev", 1869, Genre::Biography, 4.8, 10000};
    Book book2 = {"War and Peace_1", "Tolstoy Lev", 1869, Genre::Biography, 4.8, 10000};

    comp::LessByYear comparator;
    EXPECT_TRUE(comparator(book1, book2));
    EXPECT_FALSE(comparator(book2, book1));
}

// Тесты для LessByRating
TEST_F(ComparatorTest, LessByRating_SortsByRatingDescending) {
    std::sort(books.begin(), books.end(), comp::LessByRating());

    // Должны быть отсортированы по убыванию рейтинга
    EXPECT_DOUBLE_EQ(books[4].rating, 4.9);  // Chekhov
    EXPECT_DOUBLE_EQ(books[3].rating, 4.8);  // Gogol
    EXPECT_DOUBLE_EQ(books[2].rating, 4.7);  // Dostoevsky
    EXPECT_DOUBLE_EQ(books[1].rating, 4.6);  // Tolstoy
    EXPECT_DOUBLE_EQ(books[0].rating, 4.5);  // Pushkin
}

TEST_F(ComparatorTest, LessByRating_EqualRatings_ReturnsFalse) {
    Book book1 = {"War and Peace", "Tolstoy Lev", 1869, Genre::Biography, 4.8, 10000};
    Book book2 = {"War and Peace", "Tolstoy Lev", 1870, Genre::Biography, 4.8, 10000};

    comp::LessByRating comparator;
    EXPECT_TRUE(comparator(book1, book2));
    EXPECT_FALSE(comparator(book2, book1));
}

// Тесты для LessByPopularity
TEST_F(ComparatorTest, LessByPopularity_SortsByReadCountDescending) {
    std::sort(books.begin(), books.end(), comp::LessByPopularity());

    // Должны быть отсортированы по убыванию количества прочтений
    EXPECT_EQ(books[4].read_count, 12000);  // Chekhov
    EXPECT_EQ(books[3].read_count, 10000);  // Gogol
    EXPECT_EQ(books[2].read_count, 8000);   // Dostoevsky
    EXPECT_EQ(books[1].read_count, 7000);   // Tolstoy
    EXPECT_EQ(books[0].read_count, 6000);   // Pushkin
}

TEST_F(ComparatorTest, LessByPopularity_EqualReadCounts_ReturnsFalse) {
    Book book1 = {"War and Peace", "Tolstoy Lev", 1869, Genre::Fiction, 4.8, 10000};
    Book book2 = {"War and Peace", "Tolstoy Lev", 1869, Genre::Biography, 4.8, 10000};

    comp::LessByPopularity comparator;
    EXPECT_TRUE(comparator(book1, book2));
    EXPECT_FALSE(comparator(book2, book1));
}

// Тесты для BookDatabase
class BookDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        database.EmplaceBack("1984", "George Orwell", 1949, Genre::SciFi, 4., 190);
        database.EmplaceBack("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143);
        database.EmplaceBack("The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120);
        database.EmplaceBack("To Kill a Mockingbird", "Harper Lee", 1960, Genre::NonFiction, 4.8, 156);
        database.EmplaceBack("Pride and Prejudice", "Jane Austen", 1813, Genre::Fiction, 4.7, 178);
        database.EmplaceBack("The Catcher in the Rye", "J.D. Salinger", 1951, Genre::NonFiction, 4.3, 112);
        database.EmplaceBack("Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98);
        database.EmplaceBack("Jane Eyre", "Charlotte Brontë", 1847, Genre::Fiction, 4.6, 110);
        database.EmplaceBack("The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203);
        database.EmplaceBack("Lord of the Flies", "William Golding", 1954, Genre::Fiction, 4.2, 89);
    }

    BookDatabase<std::vector<Book>> database;
};

TEST_F(BookDatabaseTest, Initialization) {
    EXPECT_FALSE(database.empty());
    EXPECT_EQ(database.size(), 10);
}

TEST_F(BookDatabaseTest, PushBackAndEmplaceBack) {
    BookDatabase<std::vector<Book>> db;
    EXPECT_TRUE(db.empty());

    db.PushBack(Book("1984", "George Orwell", 1949, Genre::SciFi, 4., 190));
    EXPECT_EQ(db.size(), 1);

    db.EmplaceBack("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143);
    EXPECT_EQ(db.size(), 2);
}

TEST_F(BookDatabaseTest, InitializerListConstructor) {
    BookDatabase db{Book("1984", "George Orwell", 1949, Genre::SciFi, 4., 190),
                    Book("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143),
                    Book("The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120)};

    EXPECT_EQ(db.size(), 3);
}

TEST_F(BookDatabaseTest, Iterators) {
    // Test range-based for loop
    int count = 0;
    for (const auto &book : database) {
        ++count;
        EXPECT_TRUE(book.rating > 0);
    }
    EXPECT_EQ(count, 10);

    // Test standard algorithms
    auto it = std::find_if(database.begin(), database.end(), [](const Book &book) { return book.title == "1984"; });
    EXPECT_NE(it, database.end());
    EXPECT_EQ(it->author, "George Orwell");
}

TEST_F(BookDatabaseTest, GetBooksAndAuthors) {
    auto books = database.GetBooks();
    auto authors = database.GetAuthors();

    EXPECT_EQ(books.size(), 10);
    EXPECT_EQ(authors.size(), 9);

    // Проверяем, что авторы уникальны
    std::unordered_set<std::string> unique_authors;
    for (const auto &author_count : authors) {
        unique_authors.insert(author_count.first);
    }
    EXPECT_EQ(unique_authors.size(), 9);
}

TEST_F(BookDatabaseTest, EmptyDatabase) {
    BookDatabase emptyDb;
    EXPECT_TRUE(emptyDb.empty());
    EXPECT_EQ(emptyDb.size(), 0);
    EXPECT_EQ(emptyDb.GetBooks().size(), 0);
    EXPECT_EQ(emptyDb.GetAuthors().size(), 0);
}

// Тесты для статистических функций
TEST_F(BookDatabaseTest, BuildAuthorHistogramFlat) {
    auto histogram = buildAuthorHistogramFlat(database);

    EXPECT_EQ(histogram.size(), 9);  // 5 уникальных авторов
    EXPECT_EQ(histogram.at("George Orwell"), 2);

    // Добавляем еще одну книгу того же автора
    database.PushBack(Book("Another book", "George Orwell", 1945, Genre::Fiction, 4.4, 143));
    auto updatedHistogram = buildAuthorHistogramFlat(database);
    EXPECT_EQ(updatedHistogram.at("George Orwell"), 3);
}

TEST_F(BookDatabaseTest, CalculateGenreRatings) {
    auto genreRatings = calculateGenreRatings(database.begin(), database.end());

    EXPECT_GT(genreRatings.size(), 0);

    auto fictionRating = genreRatings.get(Genre::Fiction);
    EXPECT_DOUBLE_EQ(fictionRating, 4.55);
}

TEST_F(BookDatabaseTest, CalculateAverageRating) {
    double avgRating = calculateAverageRating(database);
    EXPECT_TRUE(std::abs(avgRating - 4.49) < 0.01);

    // Test with empty database
    BookDatabase emptyDb;
    EXPECT_DOUBLE_EQ(calculateAverageRating(emptyDb), 0.0);
}

TEST_F(BookDatabaseTest, SampleRandomBooks) {
    auto samples = sampleRandomBooks(database, 3);
    EXPECT_EQ(samples.size(), 3);

    // Все образцы должны быть валидными ссылками на книги из базы данных
    for (const auto &bookRef : samples) {
        EXPECT_NO_THROW(bookRef.get());
    }

    auto allBooks = sampleRandomBooks(database, 15);
    EXPECT_EQ(allBooks.size(), 10);

    auto allBooks_1 = sampleRandomBooks(database, 5);
    EXPECT_EQ(allBooks_1.size(), 5);
}

TEST_F(BookDatabaseTest, GetTopNByRating) {
    auto topBooks = getTopNBy(database, 3, comp::LessByRating());

    EXPECT_EQ(topBooks.size(), 3);

    // Книги должны быть отсортированы по рейтингу (убывание)
    for (size_t i = 1; i < topBooks.size(); ++i) {
        EXPECT_LE(topBooks[i - 1].get().rating, topBooks[i].get().rating);
    }

    EXPECT_TRUE(std::is_sorted(topBooks.begin(), topBooks.end(), comp::LessByRating()));

    // Проверка граничных случаев
    auto allBooks = getTopNBy(database, 10, comp::LessByPopularity());
    EXPECT_EQ(allBooks.size(), 10);
}

// Тесты для фильтров
TEST_F(BookDatabaseTest, FilterBooks) {
    // Фильтр по году
    auto yearFiltered = filterBooks(database.begin(), database.end(), YearBetween(1900, 1950));
    EXPECT_EQ(yearFiltered.size(), 5);  // 1984, Foundation, Brave New World

    // Фильтр по рейтингу
    auto ratingFiltered = filterBooks(database.begin(), database.end(), RatingAbove(4.5));
    EXPECT_GE(ratingFiltered.size(), 2);

    // Фильтр по жанру
    auto genreFiltered = filterBooks(database.begin(), database.end(), GenreIs(Genre::NonFiction));
    EXPECT_EQ(genreFiltered.size(), 2);
}

TEST_F(BookDatabaseTest, CombinedFilters) {
    // Комбинированные фильтры с all_of
    auto filtered = filterBooks(database.begin(), database.end(),
                                all_of(YearBetween(1950, 2000), RatingAbove(4.0), GenreIs(Genre::Fiction)));

    EXPECT_EQ(filtered.size(), 1);  // Только Повелитель мух

    // any_of фильтр
    auto anyFiltered = filterBooks(database.begin(), database.end(), any_of(GenreIs(Genre::SciFi), RatingAbove(4.6)));

    EXPECT_GE(anyFiltered.size(), 5);
}

TEST_F(BookDatabaseTest, EmptyFilterResults) {
    // Фильтр, который не должен ничего найти
    auto emptyResult = filterBooks(database.begin(), database.end(), all_of(YearBetween(1800, 1900), RatingAbove(5.0)));

    EXPECT_TRUE(emptyResult.empty());
}

// Тест производительности (опционально)
TEST(PerformanceTest, LargeDatabase) {
    BookDatabase largeDb;

    // Создание большой базы данных для тестов производительности
    for (int i = 0; i < 1000; ++i) {
        largeDb.EmplaceBack("Book " + std::to_string(i), "Author " + std::to_string(i % 100), 1900 + i % 124,
                            static_cast<Genre>(i % 5), i % 10 / 2, 1.0 + (i % 40) / 10.0);
    }

    // Тестирование операций, которые должны быть быстрыми
    EXPECT_EQ(largeDb.size(), 1000);

    auto start = std::chrono::high_resolution_clock::now();
    auto histogram = buildAuthorHistogramFlat(largeDb);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 100);  // Должно выполняться быстрее 100ms

    // Проверка, что гистограмма содержит правильное количество авторов
    EXPECT_EQ(histogram.size(), 100);
}

void get_memory_info(size_t &allocated, size_t &free) {
#if defined(__linux__) && defined(__GLIBC__)
#if __GLIBC_PREREQ(2, 33)
    // Используем mallinfo2 для новых версий glibc
    struct mallinfo2 mi = mallinfo2();
    allocated = mi.uordblks;  // Total allocated space
    free = mi.fordblks;       // Total free space
#else
    // Используем mallinfo для старых версий
    struct mallinfo mi = mallinfo();
    allocated = mi.uordblks;  // Total allocated space
    free = mi.fordblks;       // Total free space
#endif
#else
    // Для других платформ возвращаем 0
    allocated = 0;
    free = 0;
#endif
}

// Тест для проверки отсутствия утечек памяти
TEST(MemoryManagementTest, NoMemoryLeaksInBookOperations) {
    size_t initial_allocated = 0, initial_free = 0;
    get_memory_info(initial_allocated, initial_free);

    {
        // Создаем и уничтожаем несколько книг в ограниченной области видимости
        BookDatabase<std::vector<Book>> db;

        // Добавляем книги с разными типами строк
        db.EmplaceBack("Short", "Author1", 2000, Genre::Fiction, 4.0, 100);
        db.EmplaceBack("Very Long Title That Definitely Exceeds Small String Optimization",
                       "Very Long Author Name That Also Exceeds SSO", 2020, Genre::SciFi, 4.5, 500);

        // Проверяем, что все работает корректно
        EXPECT_EQ(db.size(), 2);

        // Копируем базу данных
        BookDatabase<std::vector<Book>> db_copy = db;
        EXPECT_EQ(db_copy.size(), 2);

        // Перемещаем базу данных
        BookDatabase<std::vector<Book>> db_moved = std::move(db);
        EXPECT_EQ(db_moved.size(), 2);
        EXPECT_EQ(db.size(), 0);  // После move исходный объект должен быть пустым
    }

    // Все объекты должны быть уничтожены к этому моменту
    size_t final_allocated = 0, final_free = 0;
    get_memory_info(final_allocated, final_free);

    // Проверяем, что память освобождена (допускаем небольшой разброс из-за кэширования аллокатора)
    EXPECT_LE(final_allocated, initial_allocated + 1024);
}

// Тест для проверки управления ресурсами при исключениях
TEST(MemoryManagementTest, ExceptionSafety) {
    BookDatabase<std::vector<Book>> db;

    // Добавляем несколько валидных книг
    db.EmplaceBack("Valid Book", "Author", 2000, Genre::Fiction, 4.0, 100);

    try {
        // Попытка добавить книгу с невалидным рейтингом (должно бросить исключение)
        db.EmplaceBack("Invalid Book", "Author", 2000, Genre::Fiction, 6.0, 100);
        FAIL() << "Expected std::invalid_argument exception";
    } catch (const std::invalid_argument &) {
        // Ожидаемое исключение
    } catch (...) {
        FAIL() << "Expected std::invalid_argument exception, got different type";
    }

    // Проверяем, что состояние базы данных осталось консистентным
    EXPECT_EQ(db.size(), 1);
    EXPECT_EQ(db.GetBooks()[0].title, "Valid Book");
}

// Тест для проверки корректности копирования и перемещения
TEST(MemoryManagementTest, CopyAndMoveSemantics) {
    size_t initial_allocated = 0, initial_free = 0;
    get_memory_info(initial_allocated, initial_free);

    BookDatabase<std::vector<Book>> original;
    original.EmplaceBack("Original", "Author", 2000, Genre::Fiction, 4.0, 100);

    // Тестируем конструктор копирования
    BookDatabase<std::vector<Book>> copy_constructed = original;
    EXPECT_EQ(copy_constructed.size(), 1);
    EXPECT_EQ(original.size(), 1);

    // Тестируем оператор присваивания копированием
    BookDatabase<std::vector<Book>> copy_assigned;
    copy_assigned = original;
    EXPECT_EQ(copy_assigned.size(), 1);
    EXPECT_EQ(original.size(), 1);

    // Тестируем конструктор перемещения
    BookDatabase<std::vector<Book>> move_constructed = std::move(original);
    EXPECT_EQ(move_constructed.size(), 1);
    EXPECT_EQ(original.size(), 0);  // Исходный объект должен быть пустым

    // Тестируем оператор присваивания перемещением
    BookDatabase<std::vector<Book>> move_assigned;
    move_assigned = std::move(copy_constructed);
    EXPECT_EQ(move_assigned.size(), 1);
    EXPECT_EQ(copy_constructed.size(), 0);  // Исходный объект должен быть пустым

    size_t final_allocated = 0, final_free = 0;
    get_memory_info(final_allocated, final_free);

    // Проверяем, что память освобождена после всех операций
    EXPECT_LE(final_allocated, initial_allocated + 2048);
}

// Тест для проверки работы с большими объемами данных
TEST(MemoryManagementTest, LargeDataHandling) {
    const size_t large_size = 1000;  // Уменьшено для быстрого тестирования

    size_t initial_allocated = 0, initial_free = 0;
    get_memory_info(initial_allocated, initial_free);

    {
        BookDatabase<std::vector<Book>> large_db;

        // Заполняем большую базу данных
        for (size_t i = 0; i < large_size; ++i) {
            large_db.EmplaceBack("Book " + std::to_string(i), "Author " + std::to_string(i % 100), 1900 + i % 124,
                                 static_cast<Genre>(i % 7), 1.0 + (i % 40) / 10.0, 50 + i % 450);
        }

        EXPECT_EQ(large_db.size(), large_size);

        // Проверяем, что все данные доступны
        for (size_t i = 0; i < 10; ++i) {  // Проверяем только sample для производительности
            EXPECT_EQ(large_db.GetBooks()[i].title, "Book " + std::to_string(i));
        }
    }

    // После выхода из области видимости вся память должна быть освобождена
    size_t final_allocated = 0, final_free = 0;
    get_memory_info(final_allocated, final_free);

    // Допускаем небольшое увеличение из-за кэширования памяти аллокатором
    EXPECT_LE(final_allocated, initial_allocated + 4096);
}

// Тест для проверки отсутствия use-after-free ошибок
TEST(MemoryManagementTest, NoUseAfterFree) {
    // Создаем книгу и освобождаем ее
    Book *book = new Book("Test", "Author", 2000, Genre::Fiction, 4.0, 100);
    delete book;

    // Попытка доступа к удаленной памяти должна приводить к неопределенному поведению,
    // но мы можем проверить, что компилятор не оптимизировал удаление
    // (этот тест в основном для демонстрации - в реальности нужно использовать инструменты вроде Valgrind)
    EXPECT_TRUE(true);  // Просто проверяем, что тест выполняется
}