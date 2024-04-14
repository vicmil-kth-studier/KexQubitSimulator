#include "L7_vector.h"
#include <typeindex>

namespace vicmil {
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

    /*template<class T>
    class owner_ptr {
    public:
        std::weak_ptr<T> ref = std::weak_ptr<T>();
        std::weak_ptr<std::list<std::shared_ptr<T>>> list_ref = std::weak_ptr<std::list<std::shared_ptr<T>>>();
        typename std::list<std::shared_ptr<T>>::iterator it;
        std::shared_ptr<T> lock() {
            return ref.lock();
        }
        bool expired() {
            Debug("expired");
            return ref.expired();
        }
        void remove() {
            START_TRACE_FUNCTION();
            if(!list_ref.expired() && !ref.expired()) {
                Debug("erase!");
                list_ref.lock()->erase(it);
            }
            END_TRACE_FUNCTION();
        }
        std::weak_ptr<T> weak_ptr() {
            return ref;
        }
        ~owner_ptr() {
            if(ref.use_count() == 1) {
                remove();
            }
        }
        // Create new instance in owner
        template<typename... _Args>
        owner_ptr<T> make_shared(_Args&&... _args) {
            START_TRACE_FUNCTION();
            owner_ptr<T> new_ref;
            list_ref.lock()->push_back(std::make_shared<T>(_args...));
            new_ref.it = list_ref.lock()->end();
            new_ref.it--;
            new_ref.ref = *it;
            new_ref.list_ref = list_ref;
            END_TRACE_FUNCTION();
            return new_ref;
        }
    };
    template<class T>
    class Owner {
        public:
        // TODO: use pool allocator for faster performance
        std::shared_ptr<std::list<std::shared_ptr<T>>> _objects = std::make_shared<std::list<std::shared_ptr<T>>>();
        owner_ptr<T> _get_ref(typename std::list<std::shared_ptr<T>>::iterator it) {
            START_TRACE_FUNCTION();
            owner_ptr<T> new_ref;
            new_ref.it = it;
            new_ref.ref = *it;
            new_ref.list_ref = _objects;
            END_TRACE_FUNCTION();
            return new_ref;
        }
        template<typename... _Args>
        owner_ptr<T> make_shared(_Args&&... _args) {
            START_TRACE_FUNCTION();
            owner_ptr<T> new_ref;
            _objects->push_back(std::make_shared<T>(_args...));
            auto it = _objects->end();
            it--;
            END_TRACE_FUNCTION();
            return _get_ref(it);
        }
        std::vector<owner_ptr<T>> all() {
            std::vector<owner_ptr<T>> vec = {};
            vec.reserve(_objects->size());
            typename std::list<std::shared_ptr<T>>::iterator it = _objects->begin();
            while(it != _objects->end()) {
                vec.push_back(_get_ref(it));
            }
            return vec;
        }

        int size() {
            return _objects->size();
        }
        struct iterator {
            std::weak_ptr<std::list<std::shared_ptr<T>>> _list;
            typename std::list<std::shared_ptr<T>>::iterator _it;
            owner_ptr<T> operator->() {
                owner_ptr<T> new_ref;
                new_ref.it = _it;
                new_ref.ref = *_it;
                new_ref.list_ref = _list;
                return new_ref;
            }
            owner_ptr<T> operator*() {
                owner_ptr<T> new_ref;
                return operator->();
            }
            void operator++() {
                _it++;
            }
            bool operator==(const Owner::iterator& other) const {
                return _it == other._it;
            }
            bool operator!=(const Owner::iterator& other) const {
                return _it != other._it;
            }
        };
        Owner::iterator begin() {
            iterator it;
            it._list = _objects;
            it._it = _objects->begin();
            return it;
        }
        Owner::iterator end() {
            iterator it;
            it._list = _objects;
            it._it = _objects->end();
            return it;
        }
    };
    void TEST_Owner() {
        Owner<int> owner = Owner<int>();
        {
            Assert(owner.size() == 0);
            vicmil::owner_ptr<int> ptr = owner.make_shared(5);
            Assert((*ptr.lock()) == 5);
            vicmil::owner_ptr<int> ptr2 = ptr;
            (*ptr.lock()) = 10;
            Assert((*ptr2.lock()) == 10);
            Assert(owner.size() == 1);
        }
        Assert(owner.size() == 0);
    }
    AddTest(TEST_Owner);*/
}