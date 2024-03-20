#define USE_DEBUG
#define DEBUG_KEYWORDS ".,!vicmil_std_lib" 
#define TEST_KEYWORDS  ".,!vicmil_std_lib"
#include "../../source/cubecollision_include.h"

int main() {
    std::cout << "Starting!" << std::endl;
    vicmil::TestClass::run_all_tests();
    std::cout << "Finished!" << std::endl;
}