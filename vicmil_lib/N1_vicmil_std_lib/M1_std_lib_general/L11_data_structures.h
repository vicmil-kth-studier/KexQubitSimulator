#include "L10_random.h"

namespace vicmil {
template<class _DATATYPE_> 
struct _PoolAllocator {
    const int KEYSIZE = 32;
    /*union dataElem { // leads to loads of error messages for even the most basic types like vectors :(
        _DATATYPE_ data_;
        unsigned int next_free;
        dataElem() { memset( this, 0, sizeof( dataElem ) ); } // Init to zeroes
    };
    std::vector<dataElem> _data = {}; // the data that is stored
    */
    std::vector<_DATATYPE_> _data_vec = {};
    std::vector<unsigned int> _next_free_vec = {};
    std::vector<u_int32_t> _keys_vec = {}; // keys ensure that old pointers don't get access since their keys wont match
    int next_free = -1;
    int free_count = 0;
    int total_allocation_count = 0; // How many objects have been allocated(and possibly freed) since start
    int get_allocation_nr(int index) {
        Assert(_keys_vec.size() > index);
        const int other_mask = (1 << (KEYSIZE-1)) - 1;
        return _keys_vec[index] & other_mask;
    }
    void set_free(int index, bool free_) {
        if(free_) {
            const int free_mask = (1 << (KEYSIZE-1));
            _keys_vec[index] = get_allocation_nr(index) | free_mask;
        }
        else {
            _keys_vec[index] = get_allocation_nr(index);
        }
    }
    bool is_free(int index) {
        const int free_mask = (1 << (KEYSIZE-1));
        return ((_keys_vec[index] & free_mask) != 0);
    }
    void _assert_free_exists() {
        if(_keys_vec.size() == 0) {
            _keys_vec.resize(1);
            _data_vec.resize(1);
            _next_free_vec.resize(1);
            next_free = 0;
            free_count = 1;
            set_free(0, true);
        }
        else if(free_count == 0) {
            // Allocate more space!
            int current_size = _keys_vec.size();
            _keys_vec.resize(current_size*2);
            _data_vec.resize(current_size*2);
            _next_free_vec.resize(current_size*2);

            // Populate new space
            for(int i = current_size; i < current_size*2; i++) {
                _keys_vec[i] = 1;
                set_free(i, true);
                _next_free_vec[i] = i+1;
            }
            next_free = current_size;
            free_count = current_size;
        }
    }
    int allocate_free_index() {
        _assert_free_exists();
        //PrintExpr(vicmil::vec_to_str(_keys_vec));
        //PrintExpr(next_free);
        int free_index = next_free;
        next_free = _next_free_vec[free_index];
        //PrintExpr(next_free);
        free_count -= 1;
        total_allocation_count += 1;
        if(free_count != 0) {
            Assert(is_free(free_index) == true);
        }
        _keys_vec[free_index] = total_allocation_count;
        set_free(free_index, false);
        return free_index;
    }
    void free_index(int index, int key) {
        if(_keys_vec[index] == key) {
            Assert(is_free(index) == false);
            // Update keys
            set_free(index, true);

            // Remove old data
            _data_vec[index].~_DATATYPE_(); 

            // Update free data pointers
            //PrintExpr(next_free);
            _next_free_vec[index] = next_free; 
            next_free = index;
            free_count += 1;
        }
    }
    bool is_valid_key(unsigned int index, unsigned int key) {
        return (_keys_vec.size() > index) && (_keys_vec[index] == key);
    }
    _DATATYPE_& get_data(unsigned int index, unsigned int key) {
        Assert(_keys_vec.size() > index);
        Assert(key == _keys_vec[index]);
        Assert(is_free(index) == false);
        return _data_vec[index];
    }
};
template<class _DATATYPE_> 
struct pool_ptr {
    std::weak_ptr<_PoolAllocator<_DATATYPE_>> pool_allocator = std::weak_ptr<_PoolAllocator<_DATATYPE_>>();
    int index = -1;
    u_int32_t key = 0;
    _DATATYPE_& operator*() {
        Assert(!pool_allocator.expired());
        return pool_allocator.lock()->get_data(index, key);
    }
    bool operator==(pool_ptr other) {
        return index == other.index && key == other.key && equals(pool_allocator, other.pool_allocator);
    }
    bool operator!=(pool_ptr other) {
        return !(operator==(other));
    }
    bool expired() {
        return pool_allocator.expired() || !pool_allocator.lock()->is_valid_key(index, key);
    }
    void erase() {
        if(!expired()) {
            pool_allocator.lock()->free_index(index, key);
        }
    }
    inline static pool_ptr<_DATATYPE_> push(std::shared_ptr<_PoolAllocator<_DATATYPE_>> allocator, _DATATYPE_ data) {
        // Allocate data
        int index = allocator->allocate_free_index();
        allocator->_data_vec[index] = data;

        // Create pointer to data
        pool_ptr<_DATATYPE_> new_pool_ptr;
        new_pool_ptr.pool_allocator = allocator;
        new_pool_ptr.index = index;
        new_pool_ptr.key = allocator->_keys_vec[index];
        return new_pool_ptr;
    }
    pool_ptr<_DATATYPE_> push(_DATATYPE_ data) {
        if(expired()) {
            return pool_ptr<_DATATYPE_>();
        } 
        return push(pool_allocator.lock(), data);
    }
};
template<class _DATATYPE_> 
struct PoolAllocator {
    std::shared_ptr<_PoolAllocator<_DATATYPE_>> allocator = std::make_shared<_PoolAllocator<_DATATYPE_>>();
    pool_ptr<_DATATYPE_> push(_DATATYPE_ data) {
        return pool_ptr<_DATATYPE_>::push(allocator, data);
    }
    int size() {
        return allocator->_keys_vec.size();
    }
    int free_count() {
        return allocator->free_count;
    }
    int alloc_count() {
        return size() - free_count();
    }
    int get_allocation_nr(int index) {
        return allocator->get_allocation_nr(index);
    }
    struct iterator {
        std::weak_ptr<_PoolAllocator<_DATATYPE_>> _allocator = std::weak_ptr<_PoolAllocator<_DATATYPE_>>();
        int _index = -1;
        void find_next_data() {
            if(_index != -1) {
                auto allocator_ = _allocator.lock();
                while(_index < allocator_->_keys_vec.size()) {
                    if(allocator_->is_free(_index) == false) {
                        return;
                    }
                    _index++;
                }
                _index = -1;
            }
        }
        void operator++() {
            if(_index != -1) {
                _index += 1;
                find_next_data();
            }
        }
        _DATATYPE_& operator*() {
            find_next_data();
            if(_index == -1) {
                ThrowError("Could not dereference pool iterator!");
            }
            return _allocator.lock()->_data_vec[_index];
        }
        bool operator==(iterator other) {
            if(!_allocator.expired() && equals(_allocator, other._allocator)) {
                find_next_data();
                other.find_next_data();
                return (_index == other._index);
            }
            return false;
        }
        bool operator!=(iterator other) {
            return !(*this == other);
        }
        pool_ptr<_DATATYPE_> ptr() {
            if(!allocator.expired()) {
                find_next_data();
                if(_index != -1) {
                    pool_ptr<_DATATYPE_> new_pool_ptr;
                    new_pool_ptr.index = _index;
                    new_pool_ptr.key = _allocator.lock()->_keys[_index];
                    new_pool_ptr.pool_allocator = _allocator;
                    return new_pool_ptr;
                }
            }
            return pool_ptr<_DATATYPE_>();
        }
    };
    iterator begin() {
        iterator new_iterator;
        new_iterator._allocator = allocator;
        new_iterator._index = 0;
        return new_iterator;
    }
    iterator end() {
        iterator new_iterator;
        new_iterator._allocator = allocator;
        new_iterator._index = -1;
        return new_iterator;
    }
};

void TEST_PoolAllocator() {
    PoolAllocator<int> my_allocator;
    Print("Allocate ptr1");
    auto ptr1 = my_allocator.push(5);
    Assert(ptr1.index == 0);
    Print("Allocate ptr2");
    auto ptr2 = my_allocator.push(3);
    Assert(ptr2.index == 1);
    Assert(ptr1.operator*() == 5);
    Print("Allocate ptr3");
    auto ptr3 = my_allocator.push(6);
    Print("Allocate ptr4");
    auto ptr4 = my_allocator.push(7);
    Print("Deallocate ptr1");
    ptr1.erase();
    Print("Allocate ptr5");
    auto ptr5 = my_allocator.push(6);
    Assert(ptr5.index == 0);
    Print("Dellocate all ptrs");
    ptr3.erase(); // Deallocate in somewhat random order!
    ptr2.erase();
    ptr1.erase(); 
    ptr5.erase();
    ptr4.erase();
    Assert(my_allocator.size() == 4); // since we reused an index it is size 4
    Assert(my_allocator.free_count() == 4); // all should be freed at this point
    Assert(ptr3.expired() == true); // Assert that they correctly register as expired

    // Try allocating some more to see that it still works!
    Print("Allocate ptrs again");
    auto ptr1v = my_allocator.push(1);
    auto ptr2v = my_allocator.push(2);
    auto ptr3v = my_allocator.push(3);
    auto ptr4v = my_allocator.push(4);
    Assert(ptr1v.operator*() == 1);
    Assert(ptr2v.operator*() == 2);
    Assert(ptr3v.operator*() == 3);
    Assert(ptr4v.operator*() == 4);
    Assert(my_allocator.size() == 4); // since we reused an index it is size 4
    Assert(my_allocator.free_count() == 0); // all should be freed at this point
}
AddTest(TEST_PoolAllocator);




// Automatically removes itself from list when it is deleted
template<class T>
struct _ListRef {
    std::weak_ptr<std::list<T>> _list_ptr = std::weak_ptr<std::list<T>>();
    typename std::list<T>::iterator _it;
    void remove() {
        START_TRACE_FUNCTION();
        if(!_list_ptr.expired()) {
            _list_ptr.lock()->erase(_it);
            _list_ptr = std::weak_ptr<std::list<T>>();
        }
        END_TRACE_FUNCTION();
    }
    ~_ListRef() {
        remove();
    }
};
template<class T>
struct list_ptr {
    std::shared_ptr<_ListRef<T>> _ptr = std::make_shared<_ListRef<T>>();
    static list_ptr<T> _push_back(T val, std::weak_ptr<std::list<T>> list_ptr_) {
        START_TRACE_FUNCTION();
        Assert(!list_ptr_.expired());
        list_ptr_.lock()->push_back(val);
        vicmil::list_ptr<T> new_list_ref;
        new_list_ref._ptr->_list_ptr = list_ptr_;
        new_list_ref._ptr->_it = list_ptr_.lock()->end();
        new_list_ref._ptr->_it--;
        END_TRACE_FUNCTION();
        return new_list_ref;
    }
    static list_ptr<T> _push_front(T val, std::weak_ptr<std::list<T>> list_ptr_) {
        Assert(!list_ptr_.expired());
        list_ptr_.lock()->push_front(val);
        vicmil::list_ptr<T> new_list_ref;
        new_list_ref._ptr->_list_ptr = list_ptr_;
        new_list_ref._ptr->_it = list_ptr_.lock()->begin();
        return new_list_ref;
    }
    list_ptr<T> push_back(T val) {
        return _push_back(val, _ptr->_list_ptr);
    }
    list_ptr<T> push_front(T val) {
        return _push_front(val, _ptr->_list_ptr);
    }
    void remove() {
        _ptr->remove();
    }
    bool expired() {
        return _ptr->_list_ptr.expired();
    }
    T& lock() {
        Assert(!expired());
        return *_ptr->_it;
    }
};
/**
 * Objects only stay in the list as long as there are object references
 * Objects can also be deleted in advance by list references!
 * The list cannot delete individual objects unless the entire list is deleted
*/
template<class T>
struct ObjectList {
    std::shared_ptr<std::list<T>> _list = std::make_shared<std::list<T>>();
    list_ptr<T> push_back(T val) {
        START_TRACE_FUNCTION();
        list_ptr<T> ptr = list_ptr<T>::_push_back(val, _list);
        END_TRACE_FUNCTION();
        return ptr;
    }
    list_ptr<T> push_front(T val) {
        return list_ptr<T>::_push_front(val, _list);
    }
    int size() {
        return _list->size();
    }
    struct iterator {
        typename std::list<T>::iterator _it;
        T& lock() {
            return *_it;
        }
        void operator++() {
            _it++;
        }
        void operator--() {
            _it--;
        }
        bool operator==(const ObjectList::iterator& other) const {
            return _it == other._it;
        }
        bool operator!=(const ObjectList::iterator& other) const {
            return _it != other._it;
        }
    };
    ObjectList::iterator begin() {
        ObjectList::iterator it;
        it._it = _list->begin();
        return it;
    }
     ObjectList::iterator end() {
        ObjectList::iterator it;
        it._it = _list->end();
        return it;
    }
};

void TEST_ObjectList() {
    ObjectList<int> my_list = ObjectList<int>();
    {
        Debug("Push back");
        list_ptr<int> ref = my_list.push_back(3);
        Debug("Assert");
        Assert(my_list.size() == 1);
        Assert(ref.lock() == 3);
    }
    Debug("Assert size 0");
    Assert(my_list.size() == 0);

    {
        // See that you can delete non-set elements
        list_ptr<int> empty_ref;
        list_ptr<int> empty_ref2 = list_ptr<int>();
    }

    // Try removing list
    {
        Debug("Push back two references");
        list_ptr<int> ref = my_list.push_back(5);
        list_ptr<int> ref2 = my_list.push_back(7);
        Debug("Assert");
        Assert(my_list.size() == 2);
        Assert(ref.lock() == 5);
        Debug("Delete list");
        my_list = ObjectList<int>();
        Assert(ref.expired() == true);
    }
}   
AddTest(TEST_ObjectList);
}