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

#ifdef DEBUG_KEYWORDS
    const std::string __debug_keywords_raw__ = DEBUG_KEYWORDS;
#else
    const std::string __debug_keywords_raw__ = ".";
#endif

#ifdef USE_DEBUG
/**
 * Evaluates to nothing if debug is disabled
*/ 
#define IfDebug(x) x
#else
/**
 * Evaluates to nothing if debug is disabled
*/ 
#define IfDebug(x)
#endif 


/**
 * Get the filename of the current file as a string
*/
#define GetFileName vicmil::split_string(__FILE__, '/').back()

/**
 * Get info about current line as a string, contains the following info
 *  - The file name
 *  - The function name
 *  - The line number
*/
#define GetLineInfo GetFileName + " " +  __func__ + "() ln: " + std::to_string(__LINE__) + " "

/**
 * Get info about current line as a string, contains the following info
 *  - The file path(relative to cpp file where it compiled)
 *  - The function name
 *  - The line number
*/
#define GetLongLineInfo std::string(__FILE__) + " " +  __func__ + "() ln: " + std::to_string(__LINE__) + " "

/**
 * Performs cout but with more info about where it was printed such as
 *  - filename
 *  - line number
*/ 
#define Print(x) std::cout << GetFileName << ": ln" << __LINE__ << ": " << x << std::endl

/**
 * Prints the code along with what the code evaluated to
*/ 
#define PrintExpr(x) Print(#x << ": '" << x << "'")

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

/**
 * If it matches ignore format
 * Search str to find keywords
 * if the keyword starts with !, make it not ignore that str
*/
inline bool match_ignore(const std::string str, const std::vector<std::string>& keywords) {
    bool return_val = false;
    for(int i = 0; i < keywords.size(); i++) {
        std::string keyword = keywords[i];
        //Print(i << " '" << keyword << "'");
        if(keyword == "" || keyword == " ") {
            continue;
        }
        if(keyword[0] == '!') { // Reject keyword
            keyword = keyword.substr(1, std::string::npos);
            if(string_contains(str, keyword)) {
                return_val = false;
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
 * Only executes x if it is relevant
 * relevant means that the line nr, function or path matches with the keywords
 * @arg x the code that should be masked
 * @arg keywords a std::vector<std::string> with the keywords 
*/
#define IfRelevant(x, keywords) \
    { \
        static const bool __relevant__ = !vicmil::match_ignore(GetLongLineInfo, keywords); \
        if (__relevant__) { \
            x; \
        } \
    }

}

const std::vector<std::string> __debug_keywords__ = vicmil::split_string(vicmil::__debug_keywords_raw__, ',');


namespace vicmil {
#ifdef USE_DEBUG
/**
 * Disables all the logging inside a function
*/
#define DisableLogging const std::vector<std::string> __debug_keywords__ = {"."};
#else
/**
 * Disables all the logging inside a function
*/
#define DisableLogging ;
#endif 


#ifdef USE_DEBUG
/**
 * Print if debug is enabled, otherwise it evaluates to nothing
*/
#define Debug(x) IfRelevant(Print(x), __debug_keywords__)
#else
/**
 * Print if debug is enabled, otherwise it evaluates to nothing
*/
#define Debug(x) 
#endif 


#ifdef USE_DEBUG
/**
 * Assert that some statement is true, it is is not, throw an error
*/
#define Assert(x) if((x) == false) {Print("Assert failed! \n" << #x); throw;}
#else
/**
 * Assert that some statement is true, it is is not, throw an error
*/
#define Assert(x)
#endif 


#ifdef USE_DEBUG
/**
 * Assert that two numerical values are equal to each other
 * @arg v1: The first numerical value
 * @arg v2: The second numerical value
 * @arg deviance: the deviance allowed between v1 and v2
*/
#define AssertEq(v1, v2, deviance) Assert(abs(v1 - v2) < deviance)
#else
/**
 * Assert that two numerical values are equal to each other
 * @arg v1: The first numerical value
 * @arg v2: The second numerical value
 * @arg deviance: the deviance allowed between v1 and v2
*/
#define Assert(x)
#endif 

/**
 * Print an error message and then throw an error
*/
#define ThrowError(x) Print(x); throw

/**
 * Print "Not implemented yet!", then throw an error
 * Used for sections of code that has yet to be implemented
*/
#define ThrowNotImplemented() ThrowError("Not implemented yet!")

/**
 * Print the code, and then what the code evaluated to
 * @arg x: The code to use
*/
#define DebugExpr(x) Debug(#x << ": '" << x << "'")

/**
 * Print that a function has begun being executed
 *   Good for degugging code where you want to know what functions are executed
*/
#define START_TRACE_FUNCTION() Debug(std::string("start ") + __func__)

/**
 * Print that a function has exited
 *   Good for degugging code where you want to know what functions are executed
*/
#define END_TRACE_FUNCTION() Debug(std::string("exit ") + __func__)
}