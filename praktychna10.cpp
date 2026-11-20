#include <cstddef>
#include <iostream>

namespace {

constexpr std::size_t kMaxCapacity = 100000;
constexpr std::size_t kTitleLen = 64;
constexpr std::size_t kGenreLen = 32;
constexpr std::size_t kLineLen = 128;

struct Album {
    char title[kTitleLen];
    char genre[kGenreLen];
    int year;
};

struct Collection {
    Album* items;
    std::size_t size;
    std::size_t capacity;
};

struct Ledger {
    long long allocations;
    long long releases;
    long long moved;
    long long reallocations;
};

Ledger gLedger = {0, 0, 0, 0};

std::size_t lengthOf(const char* s)
{
    std::size_t n = 0;
    while (s[n] != '\0') {
        ++n;
    }
    return n;
}

void copyText(char* dst, const char* src, std::size_t cap)
{
    std::size_t i = 0;
    while (i + 1 < cap && src[i] != '\0') {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
}

bool sameText(const char* a, const char* b)
{
    std::size_t i = 0;
    while (a[i] != '\0' && a[i] == b[i]) {
        ++i;
    }
    return a[i] == b[i];
}

bool parseInt(const char* s, long long& out)
{
    std::size_t i = 0;
    bool negative = false;
    if (s[0] == '-') {
        negative = true;
        i = 1;
    }
    if (s[i] == '\0') {
        return false;
    }
    long long value = 0;
    for (; s[i] != '\0'; ++i) {
        if (s[i] < '0' || s[i] > '9') {
            return false;
        }
        value = value * 10 + (s[i] - '0');
    }
    out = negative ? -value : value;
    return true;
}

bool readLine(char* buf, std::size_t cap)
{
    return static_cast<bool>(std::cin.getline(buf, static_cast<std::streamsize>(cap)));
}

void fillAlbum(Album& a, const char* title, const char* genre, int year)
{
    copyText(a.title, title, kTitleLen);
    copyText(a.genre, genre, kGenreLen);
    a.year = year;
}

Album* allocateBlock(std::size_t n)
{
    if (n == 0) {
        return nullptr;
    }
    ++gLedger.allocations;
    return new Album[n]();
}

void releaseBlock(Album*& p)
{
    if (p != nullptr) {
        delete[] p;
        ++gLedger.releases;
        p = nullptr;
    }
}

Album* newAlbum()
{
    ++gLedger.allocations;
    return new Album();
}

void deleteAlbum(Album*& p)
{
    if (p != nullptr) {
        delete p;
        ++gLedger.releases;
        p = nullptr;
    }
}

void initCollection(Collection& c)
{
    c.items = nullptr;
    c.size = 0;
    c.capacity = 0;
}

void destroyCollection(Collection& c)
{
    releaseBlock(c.items);
    c.size = 0;
    c.capacity = 0;
}

bool reserveCapacity(Collection& c, std::size_t newCap)
{
    if (newCap <= c.capacity) {
        return true;
    }
    if (newCap > kMaxCapacity) {
        return false;
    }
    Album* nb = allocateBlock(newCap);
    if (nb == nullptr) {
        return false;
    }
    for (std::size_t i = 0; i < c.size; ++i) {
        nb[i] = c.items[i];
        ++gLedger.moved;
    }
    releaseBlock(c.items);
    c.items = nb;
    c.capacity = newCap;
    ++gLedger.reallocations;
    return true;
}

bool addAlbum(Collection& c, const Album& a, bool doubling)
{
    if (c.size == c.capacity) {
        std::size_t next = 0;
        if (doubling) {
            next = (c.capacity == 0) ? 1 : c.capacity * 2;
        } else {
            next = c.capacity + 1;
        }
        if (!reserveCapacity(c, next)) {
            return false;
        }
    }
    c.items[c.size] = a;
    ++c.size;
    return true;
}

bool removeAt(Collection& c, std::size_t idx)
{
    if (idx >= c.size) {
        return false;
    }
    if (c.size == 1) {
        destroyCollection(c);
        return true;
    }
    const std::size_t newCap = c.size - 1;
    Album* nb = allocateBlock(newCap);
    if (nb == nullptr) {
        return false;
    }
    std::size_t k = 0;
    for (std::size_t i = 0; i < c.size; ++i) {
        if (i != idx) {
            nb[k] = c.items[i];
            ++k;
            ++gLedger.moved;
        }
    }
    releaseBlock(c.items);
    c.items = nb;
    c.size = newCap;
    c.capacity = newCap;
    ++gLedger.reallocations;
    return true;
}

long long findFirstByGenre(const Collection& c, const char* genre)
{
    for (std::size_t i = 0; i < c.size; ++i) {
        if (sameText(c.items[i].genre, genre)) {
            return static_cast<long long>(i);
        }
    }
    return -1;
}

long long sumYears(const Collection& c)
{
    long long total = 0;
    for (std::size_t i = 0; i < c.size; ++i) {
        total += c.items[i].year;
    }
    return total;
}

long long newestIndex(const Collection& c)
{
    if (c.size == 0) {
        return -1;
    }
    std::size_t best = 0;
    for (std::size_t i = 1; i < c.size; ++i) {
        if (c.items[i].year > c.items[best].year) {
            best = i;
        }
    }
    return static_cast<long long>(best);
}

bool setYear(Collection& c, std::size_t idx, int year)
{
    if (idx >= c.size) {
        return false;
    }
    c.items[idx].year = year;
    return true;
}

void printCollection(const Collection& c)
{
    if (c.size == 0) {
        std::cout << "Масив порожній\n";
        return;
    }
    std::cout << "Індекс | Назва                    | Жанр         | Рік\n";
    for (std::size_t i = 0; i < c.size; ++i) {
        std::cout << i << "      | " << c.items[i].title << " | " << c.items[i].genre
                  << " | " << c.items[i].year << '\n';
    }
}

void printState(const char* label, const Collection& c)
{
    std::cout << label << ": розмір " << c.size << ", місткість " << c.capacity
              << ", виділень " << gLedger.allocations << ", звільнень " << gLedger.releases
              << ", перенесено " << gLedger.moved << ", перевиділень " << gLedger.reallocations
              << '\n';
}

long long activeBlocks()
{
    return gLedger.allocations - gLedger.releases;
}

bool runCheck(int id, const char* name, bool ok, int& passed)
{
    std::cout << "тест " << id << ": " << name << (ok ? " OK" : " ПОМИЛКА") << '\n';
    if (ok) {
        ++passed;
    }
    return ok;
}

bool testEmptyView()
{
    Collection c;
    initCollection(c);
    printCollection(c);
    bool ok = (c.size == 0) && (findFirstByGenre(c, "Рок") == -1);
    destroyCollection(c);
    return ok;
}

bool testEmptyRemove()
{
    Collection c;
    initCollection(c);
    bool ok = !removeAt(c, 0) && c.size == 0;
    destroyCollection(c);
    return ok;
}

bool testOneSearch()
{
    Collection c;
    initCollection(c);
    Album a;
    fillAlbum(a, "Thriller", "Поп", 1982);
    addAlbum(c, a, true);
    bool ok = findFirstByGenre(c, "Поп") == 0;
    destroyCollection(c);
    return ok;
}

bool testOneRemove()
{
    Collection c;
    initCollection(c);
    Album a;
    fillAlbum(a, "Thriller", "Поп", 1982);
    addAlbum(c, a, true);
    bool ok = removeAt(c, 0) && c.size == 0 && c.items == nullptr && c.capacity == 0;
    destroyCollection(c);
    return ok;
}

bool testAddToThree()
{
    Collection c;
    initCollection(c);
    Album a[4];
    fillAlbum(a[0], "A", "X", 1990);
    fillAlbum(a[1], "B", "Y", 1991);
    fillAlbum(a[2], "C", "Z", 1992);
    fillAlbum(a[3], "D", "W", 1993);
    for (int i = 0; i < 4; ++i) {
        addAlbum(c, a[i], true);
    }
    bool ok = c.size == 4 && sameText(c.items[0].title, "A") && sameText(c.items[3].title, "D");
    destroyCollection(c);
    return ok;
}

void buildThree(Collection& c)
{
    initCollection(c);
    Album a[3];
    fillAlbum(a[0], "A", "X", 1990);
    fillAlbum(a[1], "B", "Y", 1991);
    fillAlbum(a[2], "C", "Z", 1992);
    for (int i = 0; i < 3; ++i) {
        addAlbum(c, a[i], true);
    }
}

bool testRemoveFirst()
{
    Collection c;
    buildThree(c);
    bool ok = removeAt(c, 0) && c.size == 2 && sameText(c.items[0].title, "B") && sameText(c.items[1].title, "C");
    destroyCollection(c);
    return ok;
}

bool testRemoveMiddle()
{
    Collection c;
    buildThree(c);
    bool ok = removeAt(c, 1) && c.size == 2 && sameText(c.items[0].title, "A") && sameText(c.items[1].title, "C");
    destroyCollection(c);
    return ok;
}

bool testRemoveLast()
{
    Collection c;
    buildThree(c);
    bool ok = removeAt(c, 2) && c.size == 2 && sameText(c.items[0].title, "A") && sameText(c.items[1].title, "B");
    destroyCollection(c);
    return ok;
}

bool testRemoveNegative()
{
    Collection c;
    buildThree(c);
    bool ok = !removeAt(c, static_cast<std::size_t>(-1)) && c.size == 3;
    destroyCollection(c);
    return ok;
}

bool testRemoveOutOfRange()
{
    Collection c;
    buildThree(c);
    bool ok = !removeAt(c, 3) && c.size == 3;
    destroyCollection(c);
    return ok;
}

bool testFullCapacityGrows()
{
    Collection c;
    initCollection(c);
    reserveCapacity(c, 3);
    Album a;
    fillAlbum(a, "A", "X", 1990);
    addAlbum(c, a, true);
    addAlbum(c, a, true);
    addAlbum(c, a, true);
    const std::size_t before = c.capacity;
    bool added = addAlbum(c, a, true);
    bool ok = added && before == 3 && c.capacity == 6 && c.size == 4;
    destroyCollection(c);
    return ok;
}

bool testHundredClear()
{
    Collection c;
    initCollection(c);
    Album a;
    for (int i = 0; i < 100; ++i) {
        fillAlbum(a, "T", "G", 1900 + i);
        addAlbum(c, a, true);
    }
    bool ok = c.size == 100;
    while (c.size > 0) {
        if (!removeAt(c, 0)) {
            ok = false;
            break;
        }
    }
    ok = ok && c.size == 0 && c.items == nullptr;
    destroyCollection(c);
    return ok;
}

bool testRefillAfterClear()
{
    Collection c;
    initCollection(c);
    Album a;
    for (int i = 0; i < 5; ++i) {
        fillAlbum(a, "T", "G", 2000 + i);
        addAlbum(c, a, true);
    }
    destroyCollection(c);
    for (int i = 0; i < 3; ++i) {
        fillAlbum(a, "R", "H", 2010 + i);
        addAlbum(c, a, true);
    }
    bool ok = c.size == 3 && c.items[2].year == 2012;
    destroyCollection(c);
    return ok;
}

bool testAlternatingFiftyTimes()
{
    Collection c;
    initCollection(c);
    Album a;
    bool ok = true;
    for (int i = 0; i < 50; ++i) {
        fillAlbum(a, "T", "G", 1950 + i);
        addAlbum(c, a, true);
        if (c.size != 1) {
            ok = false;
        }
        if (!removeAt(c, c.size - 1)) {
            ok = false;
        }
    }
    ok = ok && c.size == 0;
    destroyCollection(c);
    return ok;
}

bool testUnsuccessfulSearch()
{
    Collection c;
    buildThree(c);
    bool ok = findFirstByGenre(c, "Джаз") == -1 && c.size == 3 && sameText(c.items[0].title, "A");
    destroyCollection(c);
    return ok;
}

bool testYearSum()
{
    Collection c;
    initCollection(c);
    Album a;
    fillAlbum(a, "A", "X", 1000);
    addAlbum(c, a, true);
    fillAlbum(a, "B", "Y", 2000);
    addAlbum(c, a, true);
    fillAlbum(a, "C", "Z", 3000);
    addAlbum(c, a, true);
    bool ok = sumYears(c) == 6000;
    destroyCollection(c);
    return ok;
}

bool testNewest()
{
    Collection c;
    initCollection(c);
    Album a;
    fillAlbum(a, "A", "X", 1969);
    addAlbum(c, a, true);
    fillAlbum(a, "B", "Y", 2013);
    addAlbum(c, a, true);
    fillAlbum(a, "C", "Z", 1982);
    addAlbum(c, a, true);
    bool ok = newestIndex(c) == 1;
    destroyCollection(c);
    return ok;
}

bool testSingleObject()
{
    Album* one = newAlbum();
    fillAlbum(*one, "Abbey Road", "Рок", 1969);
    one->year = 1970;
    bool ok = one->year == 1970 && sameText(one->title, "Abbey Road");
    deleteAlbum(one);
    ok = ok && one == nullptr;
    return ok;
}

int runAllTests()
{
    int passed = 0;
    const int total = 18;
    const long long before = activeBlocks();
    std::cout << "Автоматичні тести\n";
    runCheck(1, "перегляд порожньої структури", testEmptyView(), passed);
    runCheck(2, "видалення з порожньої структури", testEmptyRemove(), passed);
    runCheck(3, "пошук у одному елементі, індекс 0", testOneSearch(), passed);
    runCheck(4, "видалення єдиного елемента, пам'ять звільнено", testOneRemove(), passed);
    runCheck(5, "додавання до трьох елементів", testAddToThree(), passed);
    runCheck(6, "видалення першого", testRemoveFirst(), passed);
    runCheck(7, "видалення середнього", testRemoveMiddle(), passed);
    runCheck(8, "видалення останнього", testRemoveLast(), passed);
    runCheck(9, "індекс -1, відмова", testRemoveNegative(), passed);
    runCheck(10, "індекс, рівний розміру, відмова", testRemoveOutOfRange(), passed);
    runCheck(11, "заповнена місткість збільшується", testFullCapacityGrows(), passed);
    runCheck(12, "100 елементів, послідовне очищення", testHundredClear(), passed);
    runCheck(13, "власний: повторне заповнення після очищення", testRefillAfterClear(), passed);
    runCheck(14, "власний: 50 чергувань додавання і видалення", testAlternatingFiftyTimes(), passed);
    runCheck(15, "власний: невдалий пошук не змінює масив", testUnsuccessfulSearch(), passed);
    runCheck(16, "сума років", testYearSum(), passed);
    runCheck(17, "найновіший альбом", testNewest(), passed);
    runCheck(18, "динамічний об'єкт", testSingleObject(), passed);
    std::cout << "Пройдено: " << passed << " з " << total << '\n';
    const bool balanced = (activeBlocks() == before);
    std::cout << "Баланс виділень і звільнень після тестів: " << (balanced ? "OK" : "ПОМИЛКА") << '\n';
    return passed;
}

struct Measure {
    long long reallocations;
    long long moved;
    long long allocations;
};

Measure measureGrowth(std::size_t n, bool doubling)
{
    const Ledger start = gLedger;
    Collection c;
    initCollection(c);
    Album a;
    for (std::size_t i = 0; i < n; ++i) {
        fillAlbum(a, "T", "G", 1900 + static_cast<int>(i % 100));
        addAlbum(c, a, doubling);
    }
    Measure m;
    m.reallocations = gLedger.reallocations - start.reallocations;
    m.moved = gLedger.moved - start.moved;
    m.allocations = gLedger.allocations - start.allocations;
    destroyCollection(c);
    return m;
}

void runExperiment()
{
    const std::size_t sizes[] = {10, 100, 1000};
    std::cout << "Експеримент: збільшення місткості на 1 проти подвоєння\n";
    for (std::size_t s = 0; s < 3; ++s) {
        const Measure plus = measureGrowth(sizes[s], false);
        const Measure dbl = measureGrowth(sizes[s], true);
        std::cout << "додавань " << sizes[s] << '\n';
        std::cout << "    на 1:      перевиділень " << plus.reallocations << ", перенесено "
                  << plus.moved << ", виділень " << plus.allocations << '\n';
        std::cout << "    подвоєння: перевиділень " << dbl.reallocations << ", перенесено "
                  << dbl.moved << ", виділень " << dbl.allocations << '\n';
        std::cout << "    зекономлено перевиділень: " << (plus.reallocations - dbl.reallocations)
                  << ", перенесень: " << (plus.moved - dbl.moved) << '\n';
    }
    std::cout << "Баланс після експерименту: " << (activeBlocks() == 0 ? "OK" : "ПОМИЛКА") << '\n';
}

void runStateTable()
{
    Collection c;
    initCollection(c);
    Album a;
    printState("старт", c);
    const char* titles[] = {"Kind of Blue", "Abbey Road", "Thriller", "Random Access Memories", "Lemonade"};
    const char* genres[] = {"Джаз", "Рок", "Поп", "Електроніка", "Поп"};
    const int years[] = {1959, 1969, 1982, 2013, 2016};
    for (int i = 0; i < 5; ++i) {
        fillAlbum(a, titles[i], genres[i], years[i]);
        addAlbum(c, a, true);
        printState("додано", c);
    }
    removeAt(c, 1);
    printState("видалено індекс 1", c);
    removeAt(c, 0);
    printState("видалено індекс 0", c);
    destroyCollection(c);
    printState("звільнено", c);
    std::cout << "Активних блоків: " << activeBlocks() << '\n';
}

void runSingleObject()
{
    char title[kLineLen];
    char genre[kLineLen];
    char yearText[kLineLen];
    std::cout << "Назва: ";
    if (!readLine(title, kLineLen)) {
        return;
    }
    std::cout << "Жанр: ";
    if (!readLine(genre, kLineLen)) {
        return;
    }
    std::cout << "Рік: ";
    if (!readLine(yearText, kLineLen)) {
        return;
    }
    long long year = 0;
    if (!parseInt(yearText, year) || year < 1000 || year > 2100) {
        std::cout << "Некоректний рік\n";
        return;
    }
    Album* one = newAlbum();
    fillAlbum(*one, title, genre, static_cast<int>(year));
    std::cout << "Об'єкт: " << one->title << " | " << one->genre << " | " << one->year << '\n';
    std::cout << "Новий рік: ";
    if (readLine(yearText, kLineLen) && parseInt(yearText, year) && year >= 1000 && year <= 2100) {
        one->year = static_cast<int>(year);
    }
    std::cout << "Після зміни: " << one->title << " | " << one->genre << " | " << one->year << '\n';
    deleteAlbum(one);
    std::cout << "Покажчик порожній: " << (one == nullptr ? "так" : "ні") << '\n';
    std::cout << "Активних блоків: " << activeBlocks() << '\n';
}

void runVariantMenu(Collection& c)
{
    char buf[kLineLen];
    for (;;) {
        std::cout << "\nВаріант 22: Альбом (назва, жанр, рік)\n";
        std::cout << "1 - Додати альбом\n";
        std::cout << "2 - Показати таблицю\n";
        std::cout << "3 - Пошук за жанром (перший збіг)\n";
        std::cout << "4 - Сума років\n";
        std::cout << "5 - Найновіший альбом\n";
        std::cout << "6 - Видалити за індексом\n";
        std::cout << "7 - Змінити рік за індексом\n";
        std::cout << "0 - Назад\n";
        std::cout << "Оберіть пункт: ";
        if (!readLine(buf, kLineLen)) {
            return;
        }
        const char choice = buf[0];
        if (choice == '0') {
            return;
        } else if (choice == '1') {
            char title[kLineLen];
            char genre[kLineLen];
            char yearText[kLineLen];
            std::cout << "Назва: ";
            if (!readLine(title, kLineLen)) {
                return;
            }
            std::cout << "Жанр: ";
            if (!readLine(genre, kLineLen)) {
                return;
            }
            std::cout << "Рік: ";
            if (!readLine(yearText, kLineLen)) {
                return;
            }
            long long year = 0;
            if (lengthOf(title) == 0 || lengthOf(title) >= kTitleLen - 1 || lengthOf(genre) >= kGenreLen - 1) {
                std::cout << "Некоректна назва або жанр\n";
            } else if (!parseInt(yearText, year) || year < 1000 || year > 2100) {
                std::cout << "Некоректний рік\n";
            } else {
                Album a;
                fillAlbum(a, title, genre, static_cast<int>(year));
                if (addAlbum(c, a, true)) {
                    std::cout << "Додано. Розмір " << c.size << ", місткість " << c.capacity << '\n';
                } else {
                    std::cout << "Не вдалося додати\n";
                }
            }
        } else if (choice == '2') {
            printCollection(c);
        } else if (choice == '3') {
            char genre[kLineLen];
            std::cout << "Жанр: ";
            if (!readLine(genre, kLineLen)) {
                return;
            }
            const long long idx = findFirstByGenre(c, genre);
            if (idx >= 0) {
                std::cout << "Знайдено: індекс " << idx << '\n';
            } else {
                std::cout << "Не знайдено, масив не змінено\n";
            }
        } else if (choice == '4') {
            std::cout << "Сума років: " << sumYears(c) << " (елементів: " << c.size << ")\n";
        } else if (choice == '5') {
            const long long idx = newestIndex(c);
            if (idx < 0) {
                std::cout << "Масив порожній\n";
            } else {
                std::cout << "Найновіший: індекс " << idx << ", " << c.items[idx].title
                          << " (" << c.items[idx].year << ")\n";
            }
        } else if (choice == '6') {
            char idxText[kLineLen];
            std::cout << "Індекс: ";
            if (!readLine(idxText, kLineLen)) {
                return;
            }
            long long idx = 0;
            if (!parseInt(idxText, idx) || idx < 0 || !removeAt(c, static_cast<std::size_t>(idx))) {
                std::cout << "Відмова: індекс поза межами, масив не змінено\n";
            } else {
                std::cout << "Видалено. Розмір " << c.size << ", місткість " << c.capacity << '\n';
            }
        } else if (choice == '7') {
            char idxText[kLineLen];
            char yearText[kLineLen];
            std::cout << "Індекс: ";
            if (!readLine(idxText, kLineLen)) {
                return;
            }
            std::cout << "Новий рік: ";
            if (!readLine(yearText, kLineLen)) {
                return;
            }
            long long idx = 0;
            long long year = 0;
            if (!parseInt(idxText, idx) || idx < 0 || !parseInt(yearText, year) || year < 1000 || year > 2100) {
                std::cout << "Некоректні дані\n";
            } else if (!setYear(c, static_cast<std::size_t>(idx), static_cast<int>(year))) {
                std::cout << "Відмова: індекс поза межами\n";
            } else {
                std::cout << "Рік змінено\n";
            }
        } else {
            std::cout << "Невідомий пункт\n";
        }
    }
}

void printMenu()
{
    std::cout << "\nПрактична робота 10. Динамічна пам'ять\n";
    std::cout << "1 - Автоматичні тести\n";
    std::cout << "2 - Варіант 22 (альбоми)\n";
    std::cout << "3 - Експеримент місткості\n";
    std::cout << "4 - Таблиця станів\n";
    std::cout << "5 - Динамічний об'єкт\n";
    std::cout << "0 - Вихід\n";
    std::cout << "Оберіть пункт: ";
}

}

int main()
{
    char choice[kLineLen];
    Collection variant;
    initCollection(variant);
    for (;;) {
        printMenu();
        if (!readLine(choice, kLineLen)) {
            break;
        }
        const char c = choice[0];
        if (c == '0') {
            break;
        } else if (c == '1') {
            runAllTests();
        } else if (c == '2') {
            runVariantMenu(variant);
        } else if (c == '3') {
            runExperiment();
        } else if (c == '4') {
            runStateTable();
        } else if (c == '5') {
            runSingleObject();
        } else {
            std::cout << "Невідомий пункт\n";
        }
    }
    destroyCollection(variant);
    std::cout << "Фінальний стан: виділень " << gLedger.allocations << ", звільнень "
              << gLedger.releases << ", активних блоків " << activeBlocks() << '\n';
    return activeBlocks() == 0 ? 0 : 1;
}
