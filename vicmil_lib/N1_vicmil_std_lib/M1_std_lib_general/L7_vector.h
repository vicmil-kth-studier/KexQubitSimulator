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
}