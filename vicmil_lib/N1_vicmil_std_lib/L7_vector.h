#pragma once
#include "L6_basic_math.h"

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
void vec_extend(std::vector<T>& vec, std::vector<T>& vec_add){
    vec.insert(vec.end(), vec_add.begin(), vec_add.end());
}

/**
Extend one vector with another(can also be referred to as vector adding or concatenation)
extend_vec({1, 2}, {3, 4, 5}) -> {1, 2, 3, 4, 5}
@arg vec: the first vector
@arg add_vec: the vector to add to vec
*/
template <class T>
void vec_extend(std::vector<T>& vec, std::vector<T> vec_add){
    vec.insert(vec.end(), vec_add.begin(), vec_add.end());
}
}