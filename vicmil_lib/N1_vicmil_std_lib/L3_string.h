#pragma once
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
void TEST_string_replace() {
    assert(string_replace("abca", "a", "d") == "dbcd");
    assert(string_replace("abcab", "ab", "d") == "dcd");
    assert(string_replace("aaaaa", "aa", "a") == "aaa");
}
AddTest(TEST_string_replace);

/**
 * Returns how many of that specific char was found in the string
*/
int count_char_in_string(std::string& str, char char_) {
    return count(str.begin(), str.end(), char_);
}

/**
 * Convert a vector of double type to a single string
 * "{123.321, 314.0, 42.0}"
*/
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

/**
 * Takes an arbitrary type and converts it to binary, eg string of 1:s and 0:es
*/
template<class T>
std::string to_binary_str(T& value) {
    int size_in_bytes = sizeof(T);
    char* bytes = (char*)&value;
    std::string return_str;
    for(int i = 0; i < size_in_bytes; i++) {
        if(i != 0) {
            return_str += " ";
        }
        for(int j = 0; j < 8; j++) {
            if((bytes[i] & (1<<j)) == 0) {
                return_str += "0";
            }
            else {
                return_str += "1";
            }
        }
    }
    return return_str;
}

/** UTF8 is compatible with ascii, and can be stored in a string(of chars)
 * Since ascii is only 7 bytes, we can have one bit represent if it is a regular ascii
 *  or if it is a unicode character. Unicode characters can be one, two, three or four bytes
 * 
 * See https://en.wikipedia.org/wiki/UTF-8 for more info
**/ 
bool is_utf8_ascii_char(char char_) {
    return ((char)(1<<7) & char_) == 0;
}
bool is_utf8_two_byte_char(char char_) {
    return ((char)(1<<5) & char_) == 0;
}
bool is_utf8_three_byte_char(char char_) {
    return ((char)(1<<4) & char_) == 0;
}
bool is_utf8_four_byte_char(char char_) {
    return ((char)(1<<3) & char_) == 0;
}
/**
 * Returns how many bytes the next utf8 letter will be comprised of
 *  the first byte always dictates the size*/
int get_utf8_letter_size(char first_char_) {
    if(is_utf8_ascii_char(first_char_)) {
        // Just treat the char as normal ascii!
        return 1;
    }
    if(is_utf8_two_byte_char(first_char_)) {
        return 2;
    }
    if(is_utf8_three_byte_char(first_char_)) {
        return 3;
    }
    if(is_utf8_four_byte_char(first_char_)) {
        return 4;
    }
    return -1; // Invalid character
}
/**
 * Returns utf8 character at index in string, and updates the index
 *  UTF8 means different characters may be different length
*/
std::string get_next_utf8_char(const std::string& str, int& index) {
    // See https://en.wikipedia.org/wiki/UTF-8 for more info
    char next_char = str[index];
    Debug(to_binary_str(next_char));
    int char_count = get_utf8_letter_size(next_char);
    if(char_count == -1) {
        ThrowError("Error when fetching utf8, invalid character!");
    }
    std::string new_char = str.substr(index, char_count);
    index += char_count;
    return new_char;
}
void TEST_get_next_utf8_char() {
    const std::vector<std::string> indv_chars = {"z", "\u00df", "\u6c34", "\U0001f34c"};
    const std::string utf8_str = "z\u00df\u6c34\U0001f34c";
    int utf8_str_index = 0;
    for(int i = 0; i < indv_chars.size(); i++) {
        std::string u8_char = get_next_utf8_char(utf8_str, utf8_str_index);
        Assert(u8_char == indv_chars[i]);
    }
}
AddTest(TEST_get_next_utf8_char);
}
