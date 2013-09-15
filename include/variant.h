
#ifndef _variant_h_
#define _variant_h_

#include <string>
#include <sstream>
#include <typeinfo>
#include <cassert>


template <typename T>
std::string to_str(T value)
{
    std::ostringstream oss;
    oss << value;
    std::string vstr = oss.str();

    return vstr;
}

struct cstring 
{
    cstring() : m("") {}
    cstring(const char* x) : m(x) {}
    cstring(const cstring& x) : m(x.m) {}
    operator const char*()
    {
        return to_ptr();
    }
    const char* to_ptr()
    {
        return m;
    }
private:
    const char* m;
};

typedef const std::type_info& TI;
  
struct object_ptr_policy
{
    static const bool make_copy = false;
    template<typename T>
    struct call_back
    {
        typedef void* arg_type;
        typedef void* return_type;

        void* operator()(T* x, void* y) 
        {
            return NULL;
        }
    };
};

struct object_copy_policy
{
    static const bool make_copy = true;
    template<typename T>
    struct call_back
    {
        typedef void* arg_type;
        typedef void* return_type;

        void* operator()(T* x, void* y) 
        {
            return NULL;
        }
    };  
};
   
// can hold a copy of any copy constructible class
template<typename Policy> 
struct base_object 
{
    // this represents the maximum size of an Object for its copy / etc. to be optimized 
    static const unsigned int buffer_size = sizeof(double); 
  
    // used to identify empty base_object types 
    struct empty {};

    // used to hold an Object or a pointer to an Object
    union holder {
        // used fields
        char buffer[buffer_size];
        void* pointer;
        // the following fields exist to help assure alignment
        // not 100% portable but sufficient for Heron
        double unused_double;
        long unused_long;
    };
  
    // function pointer table
    struct fxn_ptr_table 
    {
        TI    (*type_info)();
        void* (*get_ptr)(holder&);
        const void* (*get_const_ptr)(const holder&);
        void  (*destructor)(holder&);
        void  (*deleter)(holder&);
        void  (*clone)(holder&, const holder&);
        void* (*call_back)(holder&, void*);
    };

    // static functions for optimized types (stored in holder::buffer)
    template<typename T, bool can_optimize>
    struct fxns 
    {
        static const bool optimized = true;
        static TI type_info()
        { 
            return typeid(T); 
        }
        static T* cast(holder& x)
        { 
            return reinterpret_cast<T*>(x.buffer);
        }
        static const T* cast(const holder& x)
        {
            return reinterpret_cast<const T*>(x.buffer);
        }
        static void* get_ptr(holder& x)
        {
            return x.buffer;
        }
        static const void* get_const_ptr(const holder& x)
        {
            return x.buffer;
        }
        static void  destructor(holder& x)
        {
            cast(x)->~T();
        }
        static void  deleter(holder& x)
        {
            destructor(x); x.pointer = NULL;
        }
        static void  clone(holder& x, const holder& y)
        {
            new(x.buffer) T(*cast(y));
        }
		
        typedef typename Policy::template call_back<T> CB;
        typedef typename CB::return_type CB_result;
        typedef typename CB::arg_type CB_arg;
		
        static CB_result call_back(holder& x, CB_arg y) 
        { 
            return CB()(cast(x), y); 
        }
    };

    // static functions for unoptimized types (pointed to by holder::pointer)
    template<typename T>
    struct fxns<T, false> 
    {
        static const bool optimized = false;
        static T* cast(holder& x)
        {
            return reinterpret_cast<T*>(x.pointer);
        }
        static const T* cast(const holder& x)
        {
            return reinterpret_cast<const T*>(x.pointer);
        }
        static TI type_info()
        {
            return typeid(T);
        }
        static void* get_ptr(holder& x)
        {
            return x.pointer;
        }
        static const void* get_const_ptr(const holder& x)
        {
            return x.pointer;
        }
        static void  destructor(holder& x)
        {
            cast(x)->~T();
        }
        static void  deleter(holder& x)
        {
            delete(cast(x));
        }
        static void  clone(holder& x, const holder& y)
        {
            x.pointer = new T(*cast(y));
        }

        typedef typename Policy::template call_back<T> CB;
        typedef typename CB::return_type CB_result;
        typedef typename CB::return_type CB_arg;
		
        static CB_result call_back(holder& x, CB_arg y)
        { 
            return CB()(cast(x), y); 
        }
    };
	
    // this creates a function pointer table which points to functions for dealing with
    // either optimized or unoptimized types. 	
    template<typename T>
    static fxn_ptr_table* get_table() 
    {
        const bool optimize = sizeof(T) <= buffer_size;
        static fxn_ptr_table static_table = {
            &fxns<T, optimize>::type_info
            , &fxns<T, optimize>::get_ptr
            , &fxns<T, optimize>::get_const_ptr
            , &fxns<T, optimize>::destructor
            , &fxns<T, optimize>::deleter
            , &fxns<T, optimize>::clone
            , &fxns<T, optimize>::call_back
        };

        return &static_table;
    }

    struct bad_object_cast
    {
        bad_object_cast(TI x, TI y) 
            : from(x), to(y) { }
	  
        TI from;
        TI to;
    };

    // constructors   
    base_object() 
    {
        table_ = get_table<empty>();
        held_.pointer = NULL;
    }

    base_object(const base_object& x) 
    {
        table_ = get_table<empty>();
        held_.pointer = NULL;
        assign(x);
    }

    template <typename T>
    base_object(const T& x) 
    {
        table_ = get_table<empty>();
        held_.pointer = NULL;
        initialize(x);
    }

    base_object(const char* x)
    {
        table_ = get_table<empty>();
        held_.pointer = NULL;
        initialize(cstring(x));
    }

    ~base_object() 
    {
        reset();
    }

    // assignment
    template<typename T>
    void initialize(const T& x) 
    {
        table_ = get_table<T>();
        if (Policy::make_copy) 
        {
            if (sizeof(T) <= buffer_size) 
            {
                new(held_.buffer) T(x);
            } 
            else 
            {
                held_.pointer = new T(x);
            }
        }
        else 
        {
            held_.pointer = reinterpret_cast<void*>(const_cast<T*>(&x));
        }
    }

    base_object& assign(const base_object& x) 
    {
        reset();
        table_ = x.table_;
        if (Policy::make_copy) 
        {
            table_->clone(held_, x.held_);
        } 
        else 
        {
            held_.pointer = x.held_.pointer;
        }

        return *this;
    }

    template<typename T>
    base_object& operator = (const T& x) 
    {
        return assign(base_object(x));
    }

    base_object& operator = (const char* x) 
    {
        return assign(base_object(cstring(x)));
    }

    base_object& operator = (const base_object& x)
    {
        return assign(x);
    }

    template<typename T>
    bool operator == (const T& x) const 
    {
        T t = cast();
        return t == x;
    }

    template<typename T>
    T& cast()
    {
        if (type_info() == typeid(T))
        {
            return *to_ptr<T>();
        }
        else
        {
            base_object<object_ptr_policy> result;
            if (type_info() == typeid(int))
            {
                int i = *to_ptr<int>();
                if (typeid(T) == typeid(unsigned int))
                {
                    result = (unsigned int) i;
                }
                else if (typeid(T) == typeid(double))
                {
                    result = (double) i;
                }
                else if (typeid(T) == typeid(float))
                {
                    result = (float) i;
                }
                else if (typeid(T) == typeid(bool))
                {
                    result = bool(i != 0);
                }
                else if (typeid(T) == typeid(std::string))
                {
                    std::string *str = new std::string(to_str(i));
                    result = *str;
                }
            }
            else if (type_info() == typeid(unsigned int))
            {
                unsigned int ui = *to_ptr<unsigned int>();
                if (typeid(T) == typeid(int))
                {
                    result = (int) ui;
                }
                else if (typeid(T) == typeid(double))
                {
                    result = (double) ui;
                }
                else if (typeid(T) == typeid(float))
                {
                    result = (float) ui;
                }
                else if (typeid(T) == typeid(bool))
                {
                    result = bool(ui != 0);
                }
                else if (typeid(T) == typeid(std::string))
                {
                    std::string *str = new std::string(to_str(ui));
                    result = *str;
                }
            }
            else if (type_info() == typeid(float))
            {
                float f = *to_ptr<float>();
                if (typeid(T) == typeid(unsigned int))
                {
                    result = (unsigned int) f;
                }
                else if (typeid(T) == typeid(int))
                {
                    result = (int) f;
                }
                else if (typeid(T) == typeid(double))
                {
                    result = (double) f;
                }
                else if (typeid(T) == typeid(std::string))
                {
                    std::string *str = new std::string(to_str(f));
                    result = *str;
                }
                else if (typeid(T) == typeid(bool))
                {
                    result = (bool) (f != 0.0f);
                }
            }
            else if (type_info() == typeid(double))
            {
                double d = *to_ptr<double>();
                if (typeid(T) == typeid(unsigned int))
                {
                    result = (unsigned int) d;
                }
                else if (typeid(T) == typeid(int))
                {
                    result = (int) d;
                }
                else if (typeid(T) == typeid(float))
                {
                    result = (float) d;
                }
                else if (typeid(T) == typeid(std::string))
                {
                    std::string *str = new std::string(to_str(d));
                    result = *str;
                }
                else if (typeid(T) == typeid(bool))
                {
                    result = (bool) (d != 0.0f);
                }
            }
            else if (type_info() == typeid(bool))
            {
                bool b = *to_ptr<bool>();
                if (typeid(T) == typeid(unsigned int))
                {
                    result = (unsigned int) (b ? 1 : 0);
                }
                else if (typeid(T) == typeid(int))
                {
                    result = (int) (b ? 1 : 0);
                }
                else if (typeid(T) == typeid(double))
                {
                    result = (double) (b ? 1 : 0);
                }
                else if (typeid(T) == typeid(float))
                {
                    result = (float) (b ? 1 : 0);
                }
                else if (typeid(T) == typeid(std::string))
                {
                    std::string *str = new std::string(b ? "true" : "false");
                    result = *str;
                }
            }
            else if (type_info() == typeid(std::string) || type_info() == typeid(cstring))
            {
                std::string s;
                if (type_info() == typeid(std::string))
                {
                    s = *to_ptr<std::string>();
                }
                else if (type_info() == typeid(cstring))
                {
                    s = *to_ptr<cstring>();
                }

                if (typeid(T) == typeid(unsigned int))
                {
                    result = (unsigned int) atoi(s.c_str());
                }
                else if (typeid(T) == typeid(int))
                {
                    result = atoi(s.c_str());
                }
                else if (typeid(T) == typeid(float))
                {
                    result = (float) atof(s.c_str());
                }
                else if (typeid(T) == typeid(double))
                {
                    result = atof(s.c_str());
                }
                else if (typeid(T) == typeid(bool))
                {
                    if (s == std::string("true"))
                        result = true;
                    else
                        result = false;
                }
            }

            return *result.to_ptr<T>();
        }
    }

    // automatic casting operator
    template<typename T>
    operator const T& ()
    {
        return cast<T>();
    }

    template<typename T>
    operator T& ()
    {
        return cast<T>();
    }

    // member functions
    TI type_info() const 
    {
        return table_->type_info();
    }

    template<typename T>
    bool is() const 
    {
        return type_info() == typeid(T) ? true : false;
    }

    template<typename T>
    T& to() 
    {
        if (!is<T>()) 
            throw bad_object_cast(type_info(), typeid(T));

        return *to_ptr<T>();
    }

    template<typename T>
    const T& to() const 
    {
        if (!is<T>()) 
            throw bad_object_cast(type_info(), typeid(T));

        return *to_ptr<T>();
    }

    template<typename T>
    T* to_ptr() 
    {
        if (Policy::make_copy) 
        {
            return reinterpret_cast<T*>(table_->get_ptr(held_));
        }
        else 
        {
            return reinterpret_cast<T*>(held_.pointer);
        }
    }

    template<typename T>
    const T* to_ptr() const
    {
        if (Policy::make_copy)
        {
            return reinterpret_cast<const T*>(table_->get_const_ptr(held_));
        }
        else
        {
            return reinterpret_cast<const T*>(held_.pointer);
        }
    }

    bool is_empty() const 
    {
        return table_ == get_table<empty>();
    }

    void reset() 
    {
        if (is_empty()) 
            return;

        if (Policy::make_copy) 
        {
            table_->deleter(held_);
        }

        table_ = get_table<empty>();
    }

    base_object* operator->() 
    {
        return this;
    }

    void* call_back(void* x) 
    {
        return table_->call_back(held_, x);
    }
	
    // fields 
    fxn_ptr_table*  table_;
    holder          held_;
};

typedef base_object<object_copy_policy> var;
typedef base_object<object_ptr_policy>  ref;

#endif
