
#define USE_DEBUG
#define DEBUG_KEYWORDS "" 
#include "../std_lib_general.h"

int main() {
    std::cout << "Starting!" << std::endl;
    vicmil::TestClass::run_all_tests({""});
    std::cout << "Finished!" << std::endl;
}