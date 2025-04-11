#include <iostream>
#include <typeinfo>
#include <cassert>
#include <utility>
#include <string>
#include <unistd.h>
#include <any>
namespace Blow
{
    class any
    {
        class holder
        {
        public:
            holder() {}
            virtual ~holder() {}
            virtual const std::type_info &Type() = 0;
            virtual holder *Clone() = 0;

        private:
        };
        template <typename T>
        class place_holder : public holder
        {
        public:
            place_holder(const T &val) : _val(val) {}
            virtual ~place_holder() {}
            virtual const std::type_info &Type()
            {
                assert(typeid(T) == typeid(_val));
                return typeid(_val);
            }
            virtual holder *Clone()
            {
                return new place_holder(_val);
            }

        public:
            T _val;
        };

    public:
        any() : _hold(nullptr) {}

        ~any()
        {
            if (_hold)
            {
                delete _hold;
                _hold = nullptr;
            }
        }

        template <class T>
        any(const T &t) : _hold(new place_holder<T>(t)) {}
        any(const any &other) : _hold(other._hold ? other._hold->Clone() : nullptr) {}
        template <class T>
        any &operator=(const T &t)
        {
            any temp(t);
            Swap(temp);
            return *this;
        }
        any &operator=(const any &other)
        {
            any temp(other);
            Swap(temp);
            return *this;
        }
        template <class T>
        T *Get()
        {
            return &((reinterpret_cast<place_holder<T> *>(_hold))->_val);
        }

        void Swap(any &other)
        {
            std::swap(_hold, other._hold);
        }

    private:
        holder *_hold;
    };
}

class Test
{
public:
    Test() { std::cout << "构造" << std::endl; }
    ~Test() { std::cout << "析构" << std::endl; }
    Test(const Test &test) { std::cout << "拷贝构造" << std::endl; }
};
int main()
{
    // any a1 = Test();
    // any a2 = Test();
    // a1 = a2;
    // any a2 = std::string("abc");
    // std::string str = *(a2.Get<std::string>());
    // std::cout << str.size() << std::endl;
    std::any a = std::make_any<std::string>("abc234");
    std::cout << *std::any_cast<std::string>(&a) << std::endl;
    while (true)
        sleep(1);
    return 0;
}