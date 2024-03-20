#pragma once
#define USE_DEBUG
#include "L2_test.h"
#include <regex>



namespace vicmil {

/**
 * Replaces each instance of a str_from to str_to inside str
 * @param str the main string
 * @param str_from the pattern string we would like to replace
 * @param str_to what we want to replace str_from with
*/
inline std::string string_replace(const std::string& str, const std::string& str_from, const std::string& str_to) {
    Debug("hello!");
    DebugExpr(str);
    std::string remaining_string = str;
    std::string new_string = "";
    while(true) {
        auto next_occurence = remaining_string.find(str_from);
        if(next_occurence == std::string::npos) {
            return new_string + remaining_string;
        }
        new_string = new_string + remaining_string.substr(0, next_occurence) + str_to;
        remaining_string = remaining_string.substr(next_occurence + str_from.size(), std::string::npos);
        DebugExpr(new_string);
        DebugExpr(remaining_string);
    }
}
TestWrapper(TEST_string_replace,
    void test() {
        assert(string_replace("abca", "a", "d") == "dbcd");
        assert(string_replace("abcab", "ab", "d") == "dcd");
        assert(string_replace("aaaaa", "aa", "a") == "aaa");
    }
);

inline std::string vec_to_string(const std::vector<double>& vec) {
    std::string out_str;
    out_str += "{ ";
    for(int i = 0; i < vec.size(); i++) {
        if(i != 0) {
            out_str += ", ";
        }
        out_str += std::to_string(vec[i]);
    }
    out_str += " }";
    return out_str;
}

std::vector<std::string> regex_find_all(std::string str, std::string regex_expr) {
    // Wrap regular expression in c++ type
    std::regex r = std::regex(regex_expr);

    // Iterate to find all matches of regex expression
    std::vector<std::string> tokens = std::vector<std::string>();
    for(std::sregex_iterator i = std::sregex_iterator(str.begin(), str.end(), r);
                            i != std::sregex_iterator();
                            ++i )
    {
        std::smatch m = *i;
        //std::cout << m.str() << " at position " << m.position() << '\n';
        tokens.push_back(m.str());
    }
    return tokens;
}

inline std::string cut_off_after_find(std::string str, std::string delimiter) {
    // Find first occurrence
    size_t found_index = str.find(delimiter);

    // Take substring before first occurance
    if(found_index != std::string::npos) {
        return str.substr(0, found_index);
    }
    return str;
}

inline std::string cut_off_after_rfind(std::string str, std::string delimiter) {
    // Find first occurrence
    size_t found_index = str.rfind(delimiter);

    // Take substring before first occurance
    if(found_index != std::string::npos) {
        return str.substr(0, found_index);
    }
    return str;
}

bool regex_match_expr(std::string str, std::string regex_expr) {
    return std::regex_match(str, std::regex(regex_expr));
}

}
