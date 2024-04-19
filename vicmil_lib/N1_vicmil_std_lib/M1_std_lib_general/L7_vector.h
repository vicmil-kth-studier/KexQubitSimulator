#pragma once
#include "L6_basic_math.h"
#include <list>

namespace vicmil {

/**
 * Determine if a value exists somewhere in a vector
 * @param val The value to look for
 * @param vec The vector to look in
 * @return Returns true if value is somewhere in vector, otherwise returns false
*/
template<class T>
bool in_vector(T val, std::vector<T>& vec) {
    for(int i = 0; i < vec.size(); i++) {
        if(val == vec[i]) {
            return true;
        }
    }
    return false;
}

double get_min_in_vector(std::vector<double> vec) {
    double min_val = vec[0];
    for(int i = 0; i < vec.size(); i++) {
        if(vec[i] < min_val) {
            min_val = vec[i];
        }
    }
    return min_val;
}

double get_max_in_vector(std::vector<double> vec) {
    double max_val = vec[0];
    for(int i = 0; i < vec.size(); i++) {
        if(vec[i] > max_val) {
            max_val = vec[i];
        }
    }
    return max_val;
}

template <class T>
T vec_sum(const std::vector<T>& vec, T zero_element) {
    T sum = zero_element;
    for(int i = 0; i < vec.size(); i++) {
        sum += vec[i];
    }
    return sum;
}

template <class T>
T vec_sum(const std::vector<T>& vec) {
    return vec_sum(vec, (T)0);
}

template <class T>
void vec_sort_ascend(std::vector<T>& vec) {
    std::sort(vec.begin(), 
        vec.end(), 
        [](const T& lhs, const T& rhs) {
            return lhs < rhs;
    });
}

template <class T>
void vec_sort_descend(std::vector<T>& vec) {
    std::sort(vec.begin(), 
        vec.end(), 
        [](const T& lhs, const T& rhs) {
            return lhs > rhs;
    });
}
void TEST_vec_sort_descend() {
    std::vector<int> vec = {2, 1, 3};
    vec_sort_descend(vec);
    Assert(vec == std::vector<int>({3, 2, 1}));
}
AddTest(TEST_vec_sort_descend);


template <class T>
std::vector<std::pair<T, int>> vec_to_pair_with_indecies(const std::vector<T>& vec) {
    std::vector<std::pair<T, int>> return_vec = {};
    for(int i = 0; i < vec.size(); i++) {
        std::pair<T, int> pair_;
        pair_.first = vec[i];
        pair_.second = i;
        return_vec.push_back(pair_);
    }
    return return_vec;
}
template <class T>
std::vector<std::pair<T, int>> vec_sort_ascend_and_get_indecies(const std::vector<T>& vec) {
    std::vector<std::pair<T, int>> return_vec = vec_to_pair_with_indecies(vec);
    std::sort(return_vec.begin(), 
        return_vec.end(), 
        [](const std::pair<T, int>& lhs, const std::pair<T, int>& rhs) {
            return lhs.first < rhs.first;
    });
    return return_vec;
}
template <class T>
std::vector<std::pair<T, int>> vec_sort_descend_and_get_indecies(const std::vector<T>& vec) {
    std::vector<std::pair<T, int>> return_vec = vec_to_pair_with_indecies(vec);
    std::sort(return_vec.begin(), 
        return_vec.end(), 
        [](const std::pair<T, int>& lhs, const std::pair<T, int>& rhs) {
            return lhs.first > rhs.first;
    });
    return return_vec;
}
void TEST_vec_sort_descend_and_get_indecies() {
    std::vector<int> vec = {2, 1, 3};
    std::vector<std::pair<int, int>> vec2 = vec_sort_descend_and_get_indecies(vec);
    std::vector<std::pair<int, int>> expected_vec = std::vector<std::pair<int, int>>({
                                    std::make_pair<int, int>(3, 2), 
                                    std::make_pair<int, int>(2, 0), 
                                    std::make_pair<int, int>(1, 1)});
    Assert(vec2 == expected_vec);
}
AddTest(TEST_vec_sort_descend_and_get_indecies);

template <class T>
void vec_remove(std::vector<T>& vec, std::size_t pos)
{
    auto it = vec.begin();
    std::advance(it, pos);
    vec.erase(it);
}

/**
Extend one vector with another(can also be referred to as vector adding or concatenation)
extend_vec({1, 2}, {3, 4, 5}) -> {1, 2, 3, 4, 5}
@arg vec: the first vector
@arg add_vec: the vector to add to vec
*/
template <class T>
void vec_extend(std::vector<T>& vec, const std::vector<T>& vec_add){
    vec.insert(vec.end(), vec_add.begin(), vec_add.end());
}

/**
 * Removes all expired weak ptrs from a list
*/
template<class T>
void remove_expired_from_list(std::list<std::weak_ptr<T>>& list_){
    auto it = list_.begin();
    while(it != list_.end()) {
        if((*it).expired()) {
            auto it_copy = it;
            it++;
            list_.erase(it_copy);
            continue;
        }
        it++;
    }
}


template<class _DATATYPE_> 
struct _PoolAllocator {
    const int KEYSIZE = 32;
    union dataElem {
        _DATATYPE_ data_;
        unsigned int next_free;
    };
    std::vector<u_int32_t> _keys = {}; // keys ensure that old pointers don't get access since their keys wont match
    std::vector<dataElem> _data = {}; // the data that is stored
    int next_free = -1;
    int free_count = 0;
    int total_allocation_count = 0; // How many objects have been allocated(and possibly freed) since start
    void set_free(int index, bool free_) {
        if(free_) {
            const int other_mask = (1 >> (KEYSIZE-1)) - 1;
            const int free_mask = (1 >> (KEYSIZE-1));
            const int mask_sum = other_mask | free_mask;
            _keys[index] = _keys[index] & mask_sum;
        }
        else {
            const int other_mask = (1 >> (KEYSIZE-1)) - 1;
            const int free_mask = 0;
            const int mask_sum = other_mask | free_mask;
            _keys[index] = _keys[index] & mask_sum;
        }
    }
    bool is_free(int index) {
        const int free_mask = (1 >> (KEYSIZE-1));
        return (_keys[index] & free_mask != 0);
    }
    void _assert_free_exists() {
        if(_keys.size() == 0) {
            _keys.resize(1);
            _data.resize(1);
            next_free = 0;
            free_count = 1;
            set_free(0, true);
        }
        else if(free_count == 0) {
            // Allocate more space!
            int current_size = _keys.size();
            _keys.resize(current_size*2, 1);
            _data.resize(current_size*2, 1);

            // Populate new space
            for(int i = current_size; i < current_size*2; i++) {
                set_free(_keys[i], true);
                _data[i].next_free = i+1;
            }
            next_free = current_size;
            free_count = current_size;
        }
    }
    int allocate_free_index() {
        _assert_free_exists();
        int free_index = next_free;
        next_free = _data[free_index].next_free;
        free_count -= 1;
        total_allocation_count += 1;
        Assert(is_free(free_index) == true);
        keys[free_index] = allocator->total_allocation_count;
        set_free(free_index, false);
        return free_index;
    }
    int free_index(int index, int key) {
        if(_keys[index] == key) {
            Assert(is_free(index) == false);
            // Update keys
            _keys[index] += 1;
            set_free(index, true);

            // Remove old data
            _data[index].data.~_DATATYPE_(); 

            // Update free data pointers
            _data[index].next_free = next_free; 
            next_free = index;
            free_count += 1;
        }
    }
    bool is_valid_key(unsigned int index, unsigned int key) {
        return (_keys.size() > index) && (_keys[index] == key);
    }
    _DATATYPE_& get_data(unsigned int index, unsigned int key) {
        Assert(_keys.size() > index);
        Assert(key == _keys[index]);
        Assert(is_free(index) == false);
        return _data[index].data_;
    }
};
template<class _DATATYPE_> 
struct pool_ptr {
    std::weak_ptr<_PoolAllocator<_DATATYPE_>> pool_allocator = std::weak_ptr<_PoolAllocator<_DATATYPE_>>();
    int index = -1;
    u_int32_t key = 0;
    _DATATYPE_& operator*() {
        return pool_allocator.lock()->get_data(index, key);
    }
    bool expired() {
        return pool_allocator.expired() || !pool_allocator.is_valid_key(index, key);
    }
    void erase() {
        if(!expired()) {
            pool_allocator.lock()->free_index(index, key);
        }
    }
    inline static pool_ptr<_DATATYPE_> push(std::shared_ptr<_PoolAllocator<_DATATYPE_>> allocator, _DATATYPE_ data) {
        // Allocate data
        int index = allocator->allocate_free_index();
        allocator->_data[index].data_ = data;

        // Create pointer to data
        pool_ptr<_DATATYPE_> new_pool_ptr;
        new_pool_ptr.pool_allocator = allocator;
        new_pool_ptr.index = index;
        new_pool_ptr.key = allocator.keys[index];
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
        return allocator->_keys.size();
    }
    int free_count() {
        return allocator->free_count;
    }
    int alloc_count() {
        return size() - free_count();
    }
    struct iterator {
        std::weak_ptr<_PoolAllocator<_DATATYPE_>> _allocator = std::weak_ptr<_PoolAllocator<_DATATYPE_>>();
        int _index = -1;
        void find_next_data() {
            if(_index != -1) {
                auto allocator_ = _allocator.lock();
                while(_index < allocator_->keys.size()) {
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
            return _allocator.lock()->_data[_index].data_;
        }
        bool operator==(iterator& other) {
            if(!allocator.expired() && _allocator == other._allocator) {
                find_next_data();
                other.find_next_data();
                return (_index == other._index);
            }
            return false;
        }
        bool operator!=(iterator& other) {
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


/*template<class _DATATYPE_>
struct ForwardListIteratorInterface {
    virtual void operator++() = 0;
    virtual _DATATYPE_& operator*() = 0;
    virtual bool operator==() = 0;
    virtual bool operator!=() = 0;
};
template<class _ITERATOR_, class _DATATYPE_>
struct ForwardListInterface {
    virtual _ITERATOR_ begin() = 0;
    virtual _ITERATOR_ end() = 0;
    virtual void push_front(_DATATYPE_ val) = 0;
    virtual int size() = 0;
    virtual void erase(_ITERATOR_ it) = 0; // Erase iterator reference from list
};
template<class _DATATYPE_, class _ITERATOR_, class _LIST_TYPE_> 
struct list_ptr {
    struct Data {
        std::weak_ptr<_LIST_TYPE_> _list_ptr = std::weak_ptr<_LIST_TYPE_>();
        _ITERATOR_ _it;
        void erase() {
            if(!_list_ptr.expired()) {
                _it.erase();
            }
        }
        ~Data() {
            erase();
        }
    };
    std::shared_ptr<Data> = std::make_shared<Data>(); // Share data among class instances

    static list_ptr<_DATATYPE_, _ITERATOR_, _LIST_TYPE_> _push_back(_DATATYPE_ val, std::weak_ptr<_LIST_TYPE_> list_ptr_) {
        START_TRACE_FUNCTION();
        Assert(!list_ptr_.expired());
        list_ptr_.lock()->push_back(val);
        vicmil::list_ptr<_DATATYPE_, _ITERATOR_, _LIST_TYPE_> new_list_ref;
        new_list_ref.data->_list_ptr = list_ptr_;
        new_list_ref.data->_it = list_ptr_.lock()->end();
        new_list_ref.data->_it--;
        END_TRACE_FUNCTION();
        return new_list_ref;
    }
    list_ptr2<_DATATYPE_, _ITERATOR_, _LIST_TYPE_> push_back(_DATATYPE_ val) {
        return _push_back(val, data->_list_ptr);
    }
    void erase() {
        data->erase();
    }
    bool expired() {
        return data->_list_ptr.expired();
    }
    _DATATYPE_& operator*() {
        Assert(!expired());
        return *data->_it;
    }
    _DATATYPE_& lock() {
        Assert(!expired());
        return *data->_it;
    }
};
template<class _DATATYPE_, class _ITERATOR_, class _LIST_TYPE_>
struct ObjectList {
    struct Data {
        _LIST_TYPE_ _list = _LIST_TYPE_();
    };
    std::shared_ptr<Data> = std::make_shared<Data>(); // Share data among class instances
    list_ptr<_DATATYPE_, _ITERATOR_, _LIST_TYPE_> push_front(_DATATYPE_ val) {
        return list_ptr<_DATATYPE_, _ITERATOR_, _LIST_TYPE_>::_push_front(val, data->_list);
    }
    int size() {
        return data->_list.size();
    }
    ObjectList::iterator begin() {
        return data->_list.begin();
    }
     ObjectList::iterator end() {
        return data->_list.end();
    }
};*/


// Create list pointer for the standard list



// Create list pointer for pool allocator

/*template<class T>
struct list_ptr {
    class Data {
        std::weak_ptr<std::list<T>> _list_ptr = std::weak_ptr<std::list<T>>();
        typename std::list<T>::iterator _it;
    };
    std::shared_ptr<Data> data = std::make_shared<Data>(); // Share data among same class instances

    static list_ptr<T> _push_back(T val, std::weak_ptr<std::list<T>> list_ptr_) {
        START_TRACE_FUNCTION();
        Assert(!list_ptr_.expired());
        list_ptr_.lock()->push_back(val);
        vicmil::list_ptr<T> new_list_ref;
        new_list_ref.data->_list_ptr = list_ptr_;
        new_list_ref.data->_it = list_ptr_.lock()->end();
        new_list_ref.data->_it--;
        END_TRACE_FUNCTION();
        return new_list_ref;
    }
    static list_ptr<T> _push_front(T val, std::weak_ptr<std::list<T>> list_ptr_) {
        Assert(!list_ptr_.expired());
        list_ptr_.lock()->push_front(val);
        vicmil::list_ptr<T> new_list_ref;
        new_list_ref.data->_list_ptr = list_ptr_;
        new_list_ref.data->_it = list_ptr_.lock()->begin();
        return new_list_ref;
    }
    list_ptr<T> push_back(T val) {
        return _push_back(val, data->_list_ptr);
    }
    list_ptr<T> push_front(T val) {
        return _push_front(val, data->_list_ptr);
    }
    void remove() {
        if(!data->_list_ptr.expired()) {
            data->_list_ptr.lock()->erase(_it);
            data->_list_ptr = std::weak_ptr<std::list<T>>();
        }
    }
    bool expired() {
        return data->_list_ptr.expired();
    }
    T& lock() {
        Assert(!expired());
        return *data->_it;
    }
};*/

/**
 * Objects only stay in the list as long as there are object references
 * Objects can also be deleted in advance by list references!
 * The list cannot delete individual objects unless the entire list is deleted
*/
/*template<class T>
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
AddTest(TEST_ObjectList);*/
}