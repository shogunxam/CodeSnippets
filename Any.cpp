#include <iostream>
#include <vector>

//********************************************
//tAny Implementation
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <string>
#include <cassert>

using namespace std;

template <class T>
using StorageType = typename decay<T>::type;

class tAny
{
public:
    template <typename U>
    tAny(U&& value) : ptr(new tDerived<StorageType<U>>(forward<U>(value)))
    {
    }

    template <class U>
    bool is() const
    {
        typedef StorageType<U> T;

        auto derived = dynamic_cast<tDerived<T>*>(ptr);

        return derived;
    }

    template <class U>
    operator U()
    {
        return as<StorageType<U>>();
    }

    template <class U>
    StorageType<U>& as() const
    {
        typedef StorageType<U> T;

        auto derived = dynamic_cast<tDerived<T>*>(ptr);

        if (!derived)
            throw bad_cast();

        return derived->value;
    }

    tAny() : ptr(nullptr) {}

    tAny(tAny& that) : ptr(that.clone()) {}

    // support for std::move operation
    tAny(tAny&& that) : ptr(that.ptr)
    {
        that.ptr = nullptr;
    }

    tAny(const tAny& that) : ptr(that.clone()) {}

    tAny(const tAny&& that) : ptr(that.clone()) {}

    tAny& operator=(const tAny& a)
    {
        if (ptr == a.ptr)
            return *this;

        auto old_ptr = ptr;

        ptr = a.clone();

        if (old_ptr)
            delete old_ptr;

        return *this;
    }

    tAny& operator=(tAny&& a)
    {
        if (ptr == a.ptr)
            return *this;

        swap(ptr, a.ptr);

        return *this;
    }

    ~tAny()
    {
        if (ptr)
            delete ptr;
    }

private:

    struct tBase {
        virtual ~tBase() {}

        virtual tBase* clone() const = 0;
    };

    template <typename T>
    struct tDerived : tBase {
        template <typename U>
        tDerived(U&& value) : value(forward<U>(value))
        {
        }

        T value;

        tBase* clone() const
        {
            return new tDerived<T>(value);
        }
    };

    tBase* clone() const
    {
        if (ptr)
            return ptr->clone();
        else
            return nullptr;
    }

    tBase* ptr;
};
//********************************************

int main()
{
    std::vector<tAny> values = { 10, std::string("Hello world"), 5.4f};

    for( const auto& v : values) {
        if(v.is<int>()) {
            std::cout << "Integer: " << v.as<int>() << std::endl;
        } else if(v.is<std::string>()) {
            std::cout << "String: " << v.as<std::string>()  << std::endl;
        } else if(v.is<float>()) {
            std::cout << "Float: " << v.as<float>()  << std::endl;
        }
    }
}
