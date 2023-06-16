#include <iostream>
#include <utility>

#define DefineCustomPair(StructName, FirstType, FirstName, SecondType, SecondName) \
    struct StructName : public std::pair<FirstType, SecondType>                         \
    {  \
        FirstType& FirstName;\
        SecondType& SecondName;  \
        StructName(FirstType inFirstName = {}, SecondType inSecondName = {})    \
            : std::pair<FirstType, SecondType>(inFirstName, inSecondName), FirstName(first), SecondName(second)\
        {\
        }\
     private:  \
         using std::pair<FirstType, SecondType>::first;\
         using std::pair<FirstType, SecondType>::second; \
    };

DefineCustomPair(Point, int, x, int, y );
DefineCustomPair(Person, std::string, name, int, age );

int main()
{

   Point point;
   std::cout << "x: " << point.x << "  y: " <<point.y<< std::endl; 
   point.x = 1;
   point.y = 2;
   std::cout << "x: " << point.x << "  y: " <<point.y<< std::endl;
   Point point2(33,34);
   std::cout << "x: " << point2.x << "  y: " <<point2.y<< std::endl;
   Person person("John Doe", 32);
   std::cout << "Name: " << person.name << "  Age: " <<person.age<< std::endl;
   return 0;
}
