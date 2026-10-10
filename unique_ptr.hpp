#include <iostream>
namespace myImpl
{
    template<typename T>
    class unique_ptr
    {
        T* ptr;

        public:
        // Default constructor
        unique_ptr()
        {
            ptr = nullptr;
        }

        // Param constructor
        unique_ptr(T * arg)
        {
            ptr = arg;
        }

        // Copy constructor
        unique_ptr(const unique_ptr<T> &arg) = delete;

        // Move constructor
        unique_ptr(unique_ptr<T> &&arg)
        {
            ptr = std::move(arg.ptr);
            arg.ptr = nullptr;
        }

        // Copy assignment operator
        unique_ptr<T>& operator= (unique_ptr<T> &arg) = delete;

        // Move assignment operator
        unique_ptr<T>& operator= (unique_ptr<T> &&arg)
        {
            if (ptr != nullptr) {
                delete(ptr);
            }

            ptr = std::move(arg.ptr);
            arg.ptr = nullptr;
            return *this;
        }

        // Dereference operator
        T& operator *()
        {
            return *ptr;
        }

        T* operator->()
        {
            return ptr;
        }

        T* get()
        {
            return ptr;
        }

        T* release()
        {
            T* temp = ptr;
            ptr = nullptr;
            return temp;
        }

        explicit operator bool()
        {
            if (ptr != nullptr) {
                return true;
            }
            return false;
        }

        // Destructor
        ~unique_ptr()
        {
            if (ptr != nullptr) {
                delete(ptr);
            }
        }
    };

    template<typename T, typename ... Args>
    unique_ptr<T> make_unique(Args&& ...args)
    {
        myImpl::unique_ptr<T> ptr(new T(std::forward<Args>(args) ...));
        return ptr;
    }
}