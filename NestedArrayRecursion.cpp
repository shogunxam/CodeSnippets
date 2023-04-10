#include <iostream>
#include <type_traits>
#include <vector>

template <class T2>
struct is_std_vector { static const bool value=false; };

template <class T2>
struct is_std_vector<std::vector<T2> > { static const bool value=true; };

template <class T,  typename std::enable_if<std::is_same<T, int>::value, T>::type* = nullptr>
void modifyVector(T& v)
{
    v += 10;
}

template <class T,  typename std::enable_if<is_std_vector<T>::value, T>::type* = nullptr>
void modifyVector(T& v)
{
    for (auto& i : v) {
        modifyVector(i);
    }
}

template <class T,  typename std::enable_if<std::is_same<T, int>::value, T>::type* = nullptr>
void printVector(const T& v)
{
    std::cout << v << " ";
}

template <class T,  typename std::enable_if<is_std_vector<T>::value, T>::type* = nullptr>
void printVector(const T& v)
{
    for (const auto& i : v) {
        printVector(i);
    }
}


int main()
{
    std::vector<std::vector<std::vector<int>>> nestedVector2 = {{{1, 2}, {3, 4}}, {{5}, {6, 7}}};
    modifyVector(nestedVector2);
    printVector(nestedVector2);
    std::cout << std::endl;
    std::vector<std::vector<int>> nestedVector = {{1, 2}, {3, 4}, {5}, {}, {6, 7}};
    modifyVector(nestedVector);
    printVector(nestedVector);
    std::cout << std::endl;
    std::vector<int> nestedVector1 = {1, 2, 3, 4, 5,  6, 7};
    modifyVector(nestedVector1);
    printVector(nestedVector1);
    return 0;
}
