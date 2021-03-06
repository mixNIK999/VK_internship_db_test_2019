#include <vector>
#include <algorithm>
#include <cstddef>
#include <cstdint>

// Для тестов использую Catch2 https://github.com/catchorg/Catch2
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace std;

// Стандартные хеш-таблици std::unordered_set и std::unordered_map работают с
// невероятно большой константой, поэтому пишем свою с открытой адресацией и
// минимальным необходимым функционалом.
class FastIntHashSet {
public:
    FastIntHashSet(int capacity) : _array(capacity), _status(capacity, false) {}

    void add(int element) {
        size_t i = get_index(element);
        if (!_status[i]) {
            _array[i] = element;
            _status[i] = true;
            ++_size;
        }
    }

    bool contains(int element) const {
        return _status[get_index(element)];
    }

    size_t size() const {
        return _size;
    }

    size_t capacity() const {
        return _array.size();
    }

    // Взял отсюда https://gist.github.com/badboy/6267743
    static uint32_t good_hash(uint32_t a) {
       a = (a+0x7ed55d16) + (a<<12);
       a = (a^0xc761c23c) ^ (a>>19);
       a = (a+0x165667b1) + (a<<5);
       a = (a+0xd3a2646c) ^ (a<<9);
       a = (a+0xfd7046c5) + (a<<3);
       a = (a^0xb55a4f09) ^ (a>>16);
       return a;
    }

private:
    vector<int> _array;
    vector<char> _status; // vector<char> работает быстрее чем vector<bool>. Если память важна, то можно поменять.
    size_t _size = 0;

    size_t get_index(int element) const {
        int i = good_hash(element) % _array.size();
        while (_status[i] && _array[i] != element) {
            if (++i == (int)_array.size()) {
                i = 0;
            }
        }
        return i;
    }
};

// Решение с хеш-таблицей. Считаем что 0 < smaller.size() <= larger.size().
int count_intersection_by_hash(const vector<int> &smaller, const vector<int> &larger) {
    int ans = 0;

    FastIntHashSet hash_set(2 * smaller.size());

    for (auto e : smaller) {
        hash_set.add(e);
    }

    for (auto e : larger) {
        ans += hash_set.contains(e);
    }
    return ans;
}

// Простое решение. Считаем что 0 < smaller.size() <= larger.size().
int count_intersection_by_find(const vector<int> &smaller, const vector<int> &larger) {
    int ans = 0;

    // Вложенность именно такая, так как маленький массив кэшируется процессором
    for (auto e : larger) {
        ans += (find(begin(smaller), end(smaller), e) != end(smaller));
    }

    return ans;
}

// Полное решение
int count_intersection(const vector<int> &first_array, const vector<int> &second_array) {

    if (min(first_array.size(), second_array.size()) == 0) {
        return 0;
    }

    const vector<int> *smaller_ptr = &first_array;
    const vector<int> *larger_ptr = &second_array;
    if (smaller_ptr->size() > larger_ptr->size()) {
        swap(smaller_ptr, larger_ptr);
    }

    const size_t MIN_SIZE_FOR_HASH = 110; // подобрал
    if(smaller_ptr->size() < MIN_SIZE_FOR_HASH) {
        return count_intersection_by_find(*smaller_ptr, *larger_ptr);
    }

    return count_intersection_by_hash(*smaller_ptr, *larger_ptr);
}

// Тесты
// Мой первый опыт юнит тестирования на c++, так что не судите строго)

TEST_CASE( "FastIntHashSet unit tests", "[FastIntHashSet]" ) {

    FastIntHashSet h_table(1337);

    REQUIRE(h_table.size() == 0);
    REQUIRE(h_table.capacity() == 1337);

    SECTION("check size after add new elements") {
        int n = 10;
        for (int i = 0; i < n; i++) {
            h_table.add(i);
            REQUIRE(h_table.size() == i + 1);
        }
    }

    SECTION("check size after add the same elements") {
        int n = h_table.capacity() + 10;
        int element = 1;
        h_table.add(element);
        REQUIRE(h_table.size() == 1);
        for (int i = 0; i < n; i++) {
            h_table.add(element);
            REQUIRE(h_table.size() == 1);
        }
    }

    SECTION("contains when add a few elements") {
        int n = 10;
        for (int i = -n; i < n; i++) {
            REQUIRE(h_table.contains(i) == false);
            h_table.add(i);
            REQUIRE(h_table.contains(i) == true);
        }

        for (int i = -n; i < n; i++) {
            REQUIRE(h_table.contains(i) == true);
        }
    }

    SECTION("contains when add a lot of elements") {
        int n = (h_table.capacity() - 1) / 2;
        for (int i = -n; i < n; i++) {
            REQUIRE(h_table.contains(i) == false);
            h_table.add(i);
            REQUIRE(h_table.contains(i) == true);
        }

        for (int i = -n; i < n; i++) {
            REQUIRE(h_table.contains(i) == true);
        }
    }
}

TEST_CASE("count_intersection unit tests", "[count_intersection]") {

    SECTION("intersect two empty vectors") {
        vector<int> v1, v2;
        REQUIRE(count_intersection(v1, v2) == 0);
    }

    SECTION("intersect an empty vector and not") {
        vector<int> v1, v2 = {1, 2, 3, 4, 5};
        REQUIRE(count_intersection(v1, v2) == 0);
    }

    SECTION("intersect small equal vectors") {
        vector<int> v1 = {1, 2, 3, 4, 5};
        vector<int> v2(v1);

        REQUIRE(count_intersection(v1, v2) == 5);
        REQUIRE(count_intersection_by_find(v1, v2) == 5);
        REQUIRE(count_intersection_by_hash(v1, v2) == 5);
    }

    SECTION("empty intersect small vectors") {
        vector<int> v1 = {1, 2, 3, 4, 5};
        vector<int> v2 = {-1, -2, -3, -4, -5};

        REQUIRE(count_intersection(v1, v2) == 0);
        REQUIRE(count_intersection_by_find(v1, v2) == 0);
        REQUIRE(count_intersection_by_hash(v1, v2) == 0);
    }

    SECTION("intersect small not equal not sort vectors") {
        vector<int> smaller = {-3, -2, -1, 0};
        vector<int> larger = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        random_shuffle(begin(smaller), end(smaller));
        random_shuffle(begin(larger), end(larger));

        REQUIRE(count_intersection(smaller, larger) == 2);
        REQUIRE(count_intersection(larger, smaller) == 2);
        REQUIRE(count_intersection_by_find(smaller, larger) == 2);
        REQUIRE(count_intersection_by_hash(smaller, larger) == 2);
    }

    SECTION("intersect big not equal not sort vectors") {
        int n = 1e4;
        int m = 1e5;
        vector<int> smaller(n);
        vector<int> larger(m);

        for (int i = 0; i < n; i++) {
            smaller[i] = i - 10;
        }
        for (int i = 0; i < m; i++) {
            larger[i] = 10 - i;
        }

        random_shuffle(begin(smaller), end(smaller));
        random_shuffle(begin(larger), end(larger));

        REQUIRE(count_intersection(smaller, larger) == 21);
        REQUIRE(count_intersection(larger, smaller) == 21);
        REQUIRE(count_intersection_by_find(smaller, larger) == 21);
        REQUIRE(count_intersection_by_hash(smaller, larger) == 21);
    }

    SECTION("intersect big and small vector") {
        int m = 1e6;
        vector<int> smaller = {-1, 0, 1, 2, 3, 40, 50, 60};
        vector<int> larger(m);

        for (int i = 0; i < m; i++) {
            larger[i] = 10 - i;
        }

        random_shuffle(begin(smaller), end(smaller));
        random_shuffle(begin(larger), end(larger));

        REQUIRE(count_intersection(smaller, larger) == 5);
        REQUIRE(count_intersection(larger, smaller) == 5);
        REQUIRE(count_intersection_by_find(smaller, larger) == 5);
        REQUIRE(count_intersection_by_hash(smaller, larger) == 5);
    }
}

// Случайные тесты

vector<int> generator (mt19937 gen, uniform_int_distribution<int> uid, int size) {
    vector<int> res(size);
    set<int> was;

    for (int i = 0; i < size;) {
        int a = uid(gen);
        if (!was.count(a)) {
            was.insert(a);
            res[i] = a;
            i++;
        }
    }
    return res;
}

TEST_CASE("count_intersection stress", "[count_intersection][stress]") {

    mt19937 gen(0);
    const int MAX = 1e9;

    SECTION("Small tests") {
        int number_of_tests = 1000;
        uniform_int_distribution<int> uid(-MAX, MAX);
        vector<int> smaller;
        vector<int> larger;
        for (int t = 0; t < number_of_tests; t++) {
            smaller = generator(gen, uid, 10);
            larger = generator(gen, uid, 50);

            int by_hash = count_intersection_by_hash(smaller, larger);
            int by_find = count_intersection_by_find(smaller, larger);
            int main = count_intersection(smaller, larger);
            int rev_main = count_intersection(larger, smaller);

            REQUIRE(by_hash == by_find);
            REQUIRE(main == rev_main);
            REQUIRE(main == by_find);
        }
    }

    SECTION("Big tests") {
        int number_of_tests = 50;
        uniform_int_distribution<int> uid(-MAX, MAX);
        vector<int> smaller;
        vector<int> larger;
        for (int t = 0; t < number_of_tests; t++) {
            smaller = generator(gen, uid, 1000);
            larger = generator(gen, uid, 10000);

            int by_hash = count_intersection_by_hash(smaller, larger);
            int by_find = count_intersection_by_find(smaller, larger);
            int main = count_intersection(smaller, larger);
            int rev_main = count_intersection(larger, smaller);

            REQUIRE(by_hash == by_find);
            REQUIRE(main == rev_main);
            REQUIRE(main == by_find);
        }
    }

    SECTION("Huge tests") {
        int number_of_tests = 10;
        uniform_int_distribution<int> uid(-MAX, MAX);
        vector<int> smaller;
        vector<int> larger;
        for (int t = 0; t < number_of_tests; t++) {
            smaller = generator(gen, uid, 10000);
            larger = generator(gen, uid, 100000);

            int by_hash = count_intersection_by_hash(smaller, larger);
            int by_find = count_intersection_by_find(smaller, larger);
            int main = count_intersection(smaller, larger);
            int rev_main = count_intersection(larger, smaller);

            REQUIRE(by_hash == by_find);
            REQUIRE(main == rev_main);
            REQUIRE(main == by_find);
        }
    }

    SECTION("Small and big vectors tests") {
        int number_of_tests = 50;
        uniform_int_distribution<int> uid(-MAX, MAX);
        vector<int> smaller;
        vector<int> larger;
        for (int t = 0; t < number_of_tests; t++) {
            smaller = generator(gen, uid, 20);
            larger = generator(gen, uid, 100000);

            int by_hash = count_intersection_by_hash(smaller, larger);
            int by_find = count_intersection_by_find(smaller, larger);
            int main = count_intersection(smaller, larger);
            int rev_main = count_intersection(larger, smaller);

            REQUIRE(by_hash == by_find);
            REQUIRE(main == rev_main);
            REQUIRE(main == by_find);
        }
    }
}

// Проверка скорости. Работает только на windows.

// #include <windows.h>
//
// double PCFreq = 0.0;
// __int64 CounterStart = 0;
//
// void StartCounter() {
//     LARGE_INTEGER li;
//     QueryPerformanceFrequency(&li);
//
//     PCFreq = double(li.QuadPart)/1000.0;
//
//     QueryPerformanceCounter(&li);
//     CounterStart = li.QuadPart;
// }
//
// double GetCounter() {
//     LARGE_INTEGER li;
//     QueryPerformanceCounter(&li);
//     return double(li.QuadPart-CounterStart)/PCFreq;
// }
//
//
// int count_intersection_by_sort(const vector<int> &smaller, const vector<int> &larger) {
//     int ans = 0;
//
//     vector<int> smaller_cp(smaller);
//     sort(begin(smaller_cp), end(smaller_cp));
//
//     for (auto e : larger) {
//         auto it = lower_bound(begin(smaller_cp), end(smaller_cp), e);
//         ans +=  (it != end(larger)) && (*it == e);
//     }
//
//     return ans;
// }
//
// TEST_CASE("speed test", "[!hide][speed]") {
//
//     mt19937 gen(0);
//     const int MAX = 1e9;
//     const size_t min_const = 110;
//
//     SECTION("tests") {
//         int number_of_tests = 100;
//         uniform_int_distribution<int> uid(-MAX, MAX);
//         vector<int> smaller;
//         vector<int> larger;
//         for (int t = 0; t < number_of_tests; t++) {
//             smaller = generator(gen, uid, min_const);
//             larger = generator(gen, uid, 10000);
//
//             StartCounter();
//             int by_sort = count_intersection_by_sort(smaller, larger);
//             double time_sort = GetCounter();
//
//             StartCounter();
//             int by_find = count_intersection_by_find(smaller, larger);
//             double time_find = GetCounter();
//
//             StartCounter();
//             int by_hash = count_intersection_by_hash(smaller, larger);
//             double time_hash = GetCounter();
//
//             REQUIRE(by_sort == by_find);
//             REQUIRE(by_hash == by_find);
//             // CHECK(time_find > time_sort);
//             // CHECK(time_hash > time_sort);
//             CHECK(time_hash > time_find);
//         }
//     }
// }
