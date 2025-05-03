#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm> // For std::sort

// C++11 compatible macro to define a custom pair-like structure
#define DefineCustomPair(StructName, FirstType, FirstName, SecondType, SecondName)         \
    struct StructName : public std::pair<FirstType, SecondType>                            \
    {                                                                                      \
        typedef std::pair<FirstType, SecondType> Base;                                     \
                                                                                           \
        FirstType &FirstName;                                                              \
        SecondType &SecondName;                                                            \
                                                                                           \
        StructName()                                                                       \
            : Base(), FirstName(this->first), SecondName(this->second) {}                  \
                                                                                           \
        StructName(const FirstType &inFirst, const SecondType &inSecond)                   \
            : Base(inFirst, inSecond), FirstName(this->first), SecondName(this->second) {} \
                                                                                           \
        StructName(FirstType &&inFirst, SecondType &&inSecond)                             \
            : Base(std::move(inFirst), std::move(inSecond)),                               \
              FirstName(this->first), SecondName(this->second) {}                          \
                                                                                           \
        StructName(const StructName &other)                                                \
            : Base(other), FirstName(this->first), SecondName(this->second) {}             \
                                                                                           \
        StructName(StructName &&other)                                                     \
            : Base(std::move(other)), FirstName(this->first), SecondName(this->second) {}  \
                                                                                           \
        StructName &operator=(const StructName &other)                                     \
        {                                                                                  \
            Base::operator=(other);                                                        \
            return *this;                                                                  \
        }                                                                                  \
                                                                                           \
        StructName &operator=(StructName &&other)                                          \
        {                                                                                  \
            Base::operator=(std::move(other));                                             \
            return *this;                                                                  \
        }                                                                                  \
                                                                                           \
    private:                                                                               \
        using Base::first;                                                                 \
        using Base::second;                                                                \
    };

/******************************************************************/
/*************************** TEST *********************************/

// Definition of the custom pair structure NameAgePair
DefineCustomPair(NameAgePair, std::string, name, int, age)

    int main()
{
    std::cout << "--- Construction Test ---" << std::endl;
    NameAgePair p1("Alice", 30);
    std::cout << "p1: " << p1.name << ", " << p1.age << std::endl;

    std::cout << "\n--- Copy Test ---" << std::endl;
    NameAgePair p2 = p1;
    std::cout << "p2 (copy of p1): " << p2.name << ", " << p2.age << std::endl;

    std::cout << "\n--- Move Test ---" << std::endl;
    NameAgePair p3 = std::move(p1);
    std::cout << "p3 (moved from p1): " << p3.name << ", " << p3.age << std::endl;
    // Note: p1 might be in an indeterminate state after the move

    std::cout << "\n--- Assignment Test (Copy) ---" << std::endl;
    NameAgePair p4;
    p4 = p2;
    std::cout << "p4 (assigned from p2): " << p4.name << ", " << p4.age << std::endl;

    std::cout << "\n--- Modification Through Alias Test ---" << std::endl;
    p4.name = "Bob";
    p4.age = 40;
    std::cout << "p4 (modified): " << p4.name << ", " << p4.age << std::endl;

    std::cout << "\n--- Usage with std::vector Test ---" << std::endl;
    std::vector<NameAgePair> people;
    people.emplace_back("Charlie", 25);
    people.push_back(NameAgePair("David", 35));
    NameAgePair eve("Eve", 28);
    people.push_back(eve);

    std::cout << "Elements in the vector:" << std::endl;
    for (const auto &person : people)
    {
        std::cout << person.name << ", " << person.age << std::endl;
    }

    std::cout << "\n--- Modification of a Vector Element Test ---" << std::endl;
    if (!people.empty())
    {
        people[0].name = "Charles";
        people[0].age = 26;
        std::cout << "First element modified: " << people[0].name << ", " << people[0].age << std::endl;
    }

    std::cout << "\n--- Comparison Test (Operator == inherited from std::pair) ---" << std::endl;
    NameAgePair person1("Alice", 30);
    NameAgePair person2("Alice", 30);
    NameAgePair person3("Bob", 25);

    if (person1 == person2)
    {
        std::cout << "person1 and person2 are equal." << std::endl;
    }
    else
    {
        std::cout << "person1 and person2 are different." << std::endl;
    }

    if (person1 == person3)
    {
        std::cout << "person1 and person3 are equal." << std::endl;
    }
    else
    {
        std::cout << "person1 and person3 are different." << std::endl;
    }

    std::cout << "\n--- Comparison with std::vector Test (Search) ---" << std::endl;
    std::vector<NameAgePair> more_people;
    more_people.emplace_back("Charlie", 25);
    more_people.emplace_back("David", 35);
    more_people.emplace_back("Charlie", 25);

    NameAgePair search_person("Charlie", 25);
    int count = 0;
    for (const auto &person : more_people)
    {
        if (person == search_person)
        {
            count++;
        }
    }
    std::cout << "There are " << count << " people with name 'Charlie' and age 25 in the vector." << std::endl;

    std::cout << "\n--- Sorting Test with std::sort (Operator < inherited from std::pair) ---" << std::endl;
    std::vector<NameAgePair> unsorted_people;
    unsorted_people.emplace_back("Bob", 25);
    unsorted_people.emplace_back("Alice", 30);
    unsorted_people.emplace_back("Charlie", 20);
    unsorted_people.emplace_back("Alice", 25); // Same name, different age

    std::sort(unsorted_people.begin(), unsorted_people.end());

    std::cout << "Sorted people (by name, then by age):" << std::endl;
    for (const auto &person : unsorted_people)
    {
        std::cout << person.name << ", " << person.age << std::endl;
    }

    return 0;
}