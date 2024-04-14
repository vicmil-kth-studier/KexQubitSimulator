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