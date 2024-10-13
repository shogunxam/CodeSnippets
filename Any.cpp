#include <iostream>
#include <vector>

// Undefine this to prevent tAny from throwing an exception if comparison operators are not defined.
#define tAnyThrowException

//********************************************
// tAny Implementation
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <stdexcept>
#include <memory>

// SFINAE helpers
template<typename T, typename = void>
struct has_equal_operator : std::false_type {};

template<typename T>
struct has_equal_operator<T, typename std::enable_if<
    std::is_convertible<decltype(std::declval<T>() == std::declval<T>()), bool>::value
>::type> : std::true_type {};

template<typename T, typename = void>
struct has_less_operator : std::false_type {};

template<typename T>
struct has_less_operator<T, typename std::enable_if<
    std::is_convertible<decltype(std::declval<T>() < std::declval<T>()), bool>::value
>::type> : std::true_type {};

class tAny {
private:
    struct IBase {
        virtual ~IBase() {}
        virtual const std::type_info& type() const = 0;
        virtual std::unique_ptr<IBase> clone() const = 0;
        virtual bool equals(const IBase*) const = 0;
        virtual bool less_than(const IBase*) const = 0;
    };

    template<typename T>
    struct Derived : IBase {
        T value;

        template<typename U>
        Derived(U&& v) : value(std::forward<U>(v)) {}

        const std::type_info& type() const override { return typeid(T); }

        std::unique_ptr<IBase> clone() const override {
            return std::unique_ptr<IBase>(new Derived<T>(value));
        }

        bool equals(const IBase* other) const override {
            if (type() != other->type()) return false;
            return equal_to(static_cast<const Derived<T>*>(other)->value);
        }

        bool less_than(const IBase* other) const override {
            if (type() != other->type()) return type().before(other->type());
            return less(static_cast<const Derived<T>*>(other)->value);
        }

    private:
        template<typename U = T>
        typename std::enable_if<has_equal_operator<U>::value, bool>::type
        equal_to(const T& other) const {
            return value == other;
        }

        template<typename U = T>
        typename std::enable_if<!has_equal_operator<U>::value, bool>::type
        equal_to(const T&) const {
            #ifdef tAnyThrowException
            throw std::runtime_error("Type '" + std::string(typeid(T).name()) + "' does not support equality comparison");
            #else
            return false;
            #endif
        }

        template<typename U = T>
        typename std::enable_if<has_less_operator<U>::value, bool>::type
        less(const T& other) const {
            return value < other;
        }

        template<typename U = T>
        typename std::enable_if<!has_less_operator<U>::value, bool>::type
        less(const T&) const {
            #ifdef tAnyThrowException
            throw std::runtime_error("Type '" + std::string(typeid(T).name()) + "' does not support less than comparison");
            #else
            return false;
            #endif
        }
    };

    std::unique_ptr<IBase> ptr;

public:
    tAny() : ptr(nullptr) {}

    template<typename T, typename = typename std::enable_if<!std::is_same<typename std::decay<T>::type, tAny>::value>::type>
    tAny(T&& value) : ptr(new Derived<typename std::decay<T>::type>(std::forward<T>(value))) {}

    tAny(const tAny& other) : ptr(other.ptr ? other.ptr->clone() : nullptr) {}
    tAny(tAny&& other) noexcept = default;

    tAny& operator=(const tAny& other) {
        tAny(other).swap(*this);
        return *this;
    }

    tAny& operator=(tAny&& other) noexcept = default;

    void swap(tAny& other) noexcept {
        ptr.swap(other.ptr);
    }

    template<typename T>
    T& as() {
        if (!is<T>()) {
            throw std::bad_cast();
        }
        return static_cast<Derived<T>*>(ptr.get())->value;
    }

    template<typename T>
    const T& as() const {
        if (!is<T>()) {
            throw std::bad_cast();
        }
        return static_cast<const Derived<T>*>(ptr.get())->value;
    }

    template<typename T>
    bool is() const {
        return ptr && typeid(T) == ptr->type();
    }

    bool empty() const {
        return ptr == nullptr;
    }

    const std::type_info& type() const {
        return ptr ? ptr->type() : typeid(void);
    }

    friend bool operator==(const tAny& lhs, const tAny& rhs) {
        if (lhs.empty() && rhs.empty()) return true;
        if (lhs.empty() || rhs.empty()) return false;
        return lhs.ptr->equals(rhs.ptr.get());
    }

    friend bool operator!=(const tAny& lhs, const tAny& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator<(const tAny& lhs, const tAny& rhs) {
        if (lhs.empty()) return !rhs.empty();
        if (rhs.empty()) return false;
        return lhs.ptr->less_than(rhs.ptr.get());
    }

    friend bool operator>(const tAny& lhs, const tAny& rhs) {
        return rhs < lhs;
    }

    friend bool operator<=(const tAny& lhs, const tAny& rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>=(const tAny& lhs, const tAny& rhs) {
        return !(lhs < rhs);
    }
};
//********************************************

//********************************************
// Test Code

struct NoCompare {
    NoCompare(int v = 0) : value(v){}
    int value;
};

std::ostream& operator<<(std::ostream& ostream, const tAny& any)
{
    if(any.is<int>())
    {
        ostream << any.as<int>();
    }
    else if(any.is<std::string>())
    {
        ostream <<  any.as<std::string>();
    }
    else if(any.is<float>())
    {
        ostream <<  any.as<float>();
    }
    else if(any.is<NoCompare>())
    {
        ostream << "NoCompare::value:" << any.as<NoCompare>().value;
    }

    return ostream;
}

int main()
{
    std::vector<tAny> values = { 10, std::string("Hello world"), 5.4f, NoCompare(-1)};

    for( const auto& v : values)
    {
        if(v.is<int>())
        {
            std::cout << "Integer: " << v << std::endl;
        }
        else if(v.is<std::string>())
        {
            std::cout << "String: " << v << std::endl;
        }
        else if(v.is<float>()) 
        {
            std::cout << "Float: " << v << std::endl;
        }
        else if(v.is<NoCompare>())
        {
            std::cout << "NoCompare Class: "<< v << std::endl;;
        }
    }

    // Use try/catch in case tAnyThrowException is defined
    try
    {        
        for( int i =0; i< values.size();  i++)
        {    
                for( int j = 0; j< values.size();  j++)
                {
                    std::cout << "values[" << i << "] == values[" <<j<< "] -> " << values[i] << " == " << values[j] << " : " << (values[i] == values[j]) << std::endl;
                    std::cout << "values[" << i << "] < values[" <<j<< "] -> " << values[i] << " < " << values[j] << " : " << (values[i] < values[j]) << std::endl;
                    std::cout << "values[" << i << "] > values[" <<j<< "] -> " << values[i] << " > " << values[j] << " : " << (values[i] > values[j]) << std::endl;
                }
        }
    }
    catch(std::exception& e)
    {
        std::cout << e.what() <<std::endl;
    }
}
