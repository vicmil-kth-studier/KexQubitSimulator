#include "L7_vector.h"
#include <typeindex>

namespace vicmil {
    template <typename T, typename U>
    inline bool equals(const std::weak_ptr<T>& t, const std::weak_ptr<U>& u)
    {
        return !t.owner_before(u) && !u.owner_before(t);
    }

    template <typename T, typename U>
    inline bool equals(const std::weak_ptr<T>& t, const std::shared_ptr<U>& u)
    {
        return !t.owner_before(u) && !u.owner_before(t);
    }

    template<class T>
    std::type_index _get_type_index() {
        return std::type_index(typeid(T));
    }
    template<class T>
    std::string type_to_str() {
        std::type_index typ = _get_type_index<T>();
        std::string name = typ.name();
        return name;
    }
    template<class T>
    int64_t type_to_int() {
        std::type_index typ = _get_type_index<T>();
        std::size_t code = typ.hash_code();
        return (int64_t)code;
    }
    template<class T>
    int64_t type_to_int(T* _) {
        return type_to_int<T>();
    }
    template<class T>
    T* null_if_type_missmatch(T* v, int64_t type_int) {
        if(type_to_int<T>() == type_int) {
            return v;
        }
        return nullptr;
    }

    /**
     * Have a type that can represent any type, and then see if you can cast it!
    */
    class AnyType {
    public:
        virtual void* _get_this(int64_t class_type_int) = 0;

        // Returns nullptr if cast fails!
        template<class T>
        T* try_cast() {
            int64_t class_type_int = type_to_int<T>();
            void* this_ptr = this->_get_this(class_type_int);
            if(this_ptr != nullptr) {
                // Success! Do the cast and return result!
                return (T*)this_ptr;
            }
            return nullptr; // Failed to find cast
        }
    };

    class AnyTypeString: public AnyType {
        public:
        void* _get_this(int64_t type_int) override { return null_if_type_missmatch(this, type_int); }

        std::string str;
    };
    class AnyTypeInt: public AnyType {
        public:
        void* _get_this(int64_t type_int) override { return null_if_type_missmatch(this, type_int); }
        
        int int_;
    };
}