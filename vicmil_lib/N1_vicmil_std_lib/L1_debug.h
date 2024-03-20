#pragma once
#include <iostream>
#include <math.h>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include <fstream>
#include <filesystem>

namespace vicmil {

/**
 * Split a string for each separator character into a vector
 * @param str The string to split
 * @param separator The separator that splits the string
 * @return The vector of substrings after the splitting
*/
inline std::vector<std::string> split_string(std::string str, char separator) {
    if(str.length() == 0) {
        return std::vector<std::string>();
    }
    
    std::vector<std::string> strings;

    int startIndex = 0, endIndex = 0;
    for (int i = 0; i <= str.size(); i++) {
        // If we reached the end of the word or the end of the input.
        if (str[i] == separator || i == str.size()) {
            endIndex = i;
            std::string temp;
            temp.append(str, startIndex, endIndex - startIndex);
            strings.push_back(temp);
            startIndex = endIndex + 1;
        }
    }

    return strings;
}

/**
 * Determine if a string contains any instances of another string
 * @param str the string to look in
 * @param substr the pattern string that we want to see if it contains
 * @return Returs true if substr could be found somewhere in str, otherwise returns false
*/
inline bool string_contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

/** Determine if a string matches the keywords
 * Returns false if it does not match any keywords
 * Returns false if it matches with any prohibited keywords, marked with ! in front
 * Otherwise, returns true
*/
inline bool match_keywords(const std::string str, const std::vector<std::string>& keywords) {
    bool return_val = false;
    for(int i = 0; i < keywords.size(); i++) {
        std::string keyword = keywords[i];
        if(keyword[0] == '!') { // Reject keyword
            keyword = keyword.substr(1, std::string::npos);
            if(string_contains(str, keyword)) {
                return false;
            }
        }
        else {
            if(string_contains(str, keyword)) {
                return_val = true;
            }
        }
    }
    return return_val;
}



/**
 * Get the filename of the current file
*/
#define GetFileName vicmil::split_string(__FILE__, '/').back()
}

#ifdef DEBUG_KEYWORDS
    const std::string __debug_keywords_raw__ = DEBUG_KEYWORDS;
#else
    const std::string __debug_keywords_raw__ = ".,!vicmil_lib";
#endif

const std::vector<std::string> __debug_keywords__ = vicmil::split_string(__debug_keywords_raw__, ',');

namespace vicmil {
#define GetLineIdentifier GetFileName + " " +  __func__ + "() ln: " + std::to_string(__LINE__) + " " // Used to see if line should be printed
#define GetLineIdentifierLong std::string(__FILE__) + " " +  __func__ + "() ln: " + std::to_string(__LINE__) + " " // Used to see if line should be printed

#define IfRelevantDebug(x) \
    { \
        static const bool __relevant__ = vicmil::match_keywords(GetLineIdentifierLong, __debug_keywords__); \
        if (__relevant__) { \
            x; \
        } \
    }

        
#ifdef USE_DEBUG
#define DisableLogging const std::vector<std::string> __debug_keywords__ = {};
#else
#define DisableLogging ;
#endif 

#ifdef USE_DEBUG
#define Debug(x) IfRelevantDebug(std::cout << GetFileName << ": ln" << __LINE__ << ": " << x << std::endl)
#else
#define Debug(x) 
#endif 

#ifdef USE_DEBUG
#define Assert(x) if((x) == false) {std::cout << GetFileName << ": ln" << __LINE__ << ": " << "Assert failed! \n" << #x << std::endl; throw;}
//#define Assert(x) assert(x)
#else
#define Assert(x)
#endif 

#define ThrowError(x) std::cout << GetFileName << ": ln" << __LINE__ << ": " << x << std::endl; throw
#define ThrowNotImplemented() std::cout << GetFileName << ": ln" << __LINE__ << ": " << "Not implemented yet!" << std::endl; throw

#define START_TRACE_FUNCTION() Debug(std::string("start ") + __func__)
#define END_TRACE_FUNCTION() Debug(std::string("exit ") + __func__)

#define DebugExpr(x) Debug(#x << ": '" << x << "'")

/**
 * Throws an error if val != expected, also prints a nice error message
*/
template <class T>
static void expect_value(T val, T expected, std::string err_message) {
    if(val != expected) {
        std::cout << err_message << ", expected " << expected << "got: " << val << std::endl;
        throw;
    }
}

/*
Convert a vector of strings into a single string
["a", "b"] -> "{'a'   'b'}"
*/
std::string str_vec_to_str(std::vector<std::string> vec) {
    std::string return_str = "{\n";
    for(int i = 0; i < vec.size(); i++) {
        return_str.append("    '" + vec[i] + "'\n");
    }
    return_str += "}";
    return return_str;
}

/**
 * Throws an error if vec.size() != size, also prints a nice error message
*/
template<class T>
void expect_vec_length(std::vector<T>& vec, int size, std::string err_message) {
    if (vec.size() != size) {
        std::cout << err_message << "expected size " << size << " got " << vec.size() << std::endl;
        std::cout << str_vec_to_str(vec);
        throw;
    }
}

}