#include <iostream>
#include <atomic>
#include <memory>

namespace myImpl
{
    template<typename T> class weak_ptr;

    struct ControlBlock
    {
        std::atomic<int> sharedCount;
        std::atomic<int> weakCount;

        ControlBlock(int sc, int wc): sharedCount(sc), weakCount(wc) {}
    };

    template<typename T>
    class shared_ptr
    {
        T* ptr;
        ControlBlock *cb;
        friend class weak_ptr<T>;

        shared_ptr(T* aptr, ControlBlock *aCb)
        {
            ptr = aptr;
            cb  = aCb;
            cb->sharedCount.fetch_add(1);
        }

    public:
        // Default constructor
        shared_ptr()
        {
            ptr = nullptr;
            cb = new ControlBlock(1, 0);
        }

        // Param constructor
        explicit shared_ptr(T* arg)
        {
            ptr = arg;
            cb = new ControlBlock(1, 0);
        }

        // Copy constructor
        shared_ptr(const shared_ptr<T>&arg)
        {
            ptr = arg.ptr;
            arg.cb->sharedCount.fetch_add(1);
            cb = arg.cb;
        }

        // Move constructor
        shared_ptr(shared_ptr<T>&&arg)
        {
            ptr  = std::move(arg.ptr);
            cb   = arg.cb;
            arg.ptr = nullptr
            cb = arg.cb;
        }

        // Copy assignment operator
        shared_ptr<T>& operator= (const shared_ptr<T> &arg)
        {
            if (arg.ptr == this.ptr) {
                return *this;
            }

            if (cb != nullptr) {
                cb->sharedCount.fetch_sub(1);
                if (cb->sharedCount.load() == 0) {
                    delete(ptr);
                    if (cb->weakCount.load() == 0) {
                        delete(cb);
                    }
                }
            }
            ptr = arg.ptr;
            arg.cb->sharedCount.fetch_add(1);
            cb = arg.cb;
            return *this;
        }

        // Move assignment operator
        shared_ptr<T>& operator= (const shared_ptr<T> &&arg)
        {
            if (arg.ptr == this.ptr) {
                return *this;
            }

            if (cb != nullptr) {
                cb->sharedCount.fetch_sub(1);
                if (cb->sharedCount.load() == 0) {
                    delete(ptr);
                    if (cb->weakCount.load() == 0) {
                        delete(cb);
                    }
                }
            }
            ptr = std::move(arg.ptr);
            arg.ptr = nullptr;
            cb = arg.cb;
            arg.cb = nullptr;
            return *this;
        }

        int use_count()
        {
            return cb->sharedCount.load();
        }

        T& operator *()
        {
            return *ptr;
        }

        T* operator ->()
        {
            return ptr;
        }

        T* get()
        {
            return ptr;
        }

        explicit operator bool()
        {
            if (ptr != nullptr) {
                return true;
            }
            return false;
        }

        void reset()
        {
            if (cb != nullptr) {
                cb->sharedCount.fetch_sub(1);
                if (cb->sharedCount.load() == 0) {
                    delete(ptr);
                    if (cb->weakCount.load() == 0) {
                        delete(cb);
                    }
                }
                ptr = nullptr;
                cb  = nullptr; 
            }
        }

        void reset(T* newptr)
        {
            reset();
            ptr = newptr;
            cb  = new ControlBlock(1, 0); 
        }

        // Destructor
        ~shared_ptr()
        {
            if (cb != nullptr) {
                cb->sharedCount.fetch_sub(1);
                if (cb->sharedCount.load() == 0) {
                    delete(ptr);
                    if (cb->weakCount.load() == 0) {
                        delete(cb);
                    }
                }
            }
        }
    };

    template<typename T, typename... Args>
    shared_ptr<T> make_shared(Args&& .. args)
    {
        myImpl::shared_ptr<T>ptr(new T(std::forward<Args>(args)...));
        return ptr;
    }

    template<typename T>
    class weak_ptr
    {
        T* ptr;
        ControlBlock *cb;
    public:
        weak_ptr()
        {
            ptr = nullptr;
            cb  = nullptr; 
        }

        weak_ptr(T* arg) = delete;

        weak_ptr(shared_ptr<T> &arg)
        {
            ptr = arg.ptr;
            cb  = arg.cb;
            cb->weakCount.fetch_add(1); 
        }

        weak_ptr(weak_ptr<T> &arg)
        {
            ptr = arg.ptr;
            cb  = arg.cg;
            cb->weakCount.fetch_add(1);
        }

        weak_ptr(weak_ptr<T> &&arg)
        {
            ptr = std::move(arg.ptr);
            cb  = std::move(arg.cb);
            arg.ptr = nullptr;
            arg.cb  = nullptr;
        }

        ~weak_ptr()
        {
            if (cb != nullptr) {
                cb->weakCount.fetch_sub(1);
                if (cb->weakCount.load() == 0 && cb->sharedCount.load() == 0) {
                    delete(cb);
                }
            }
        }

        int use_count()
        {
            return cb->sharedCount.load();
        }

        shared_ptr<T> lock()
        {
            if (cb != nullptr && cb->sharedCount.load() > 0) {
                return shared_ptr<T>(ptr, cb);
            }
            else {
                return shared_ptr<T>();
            }
        }

    };
}