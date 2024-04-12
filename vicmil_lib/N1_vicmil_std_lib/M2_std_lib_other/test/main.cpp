
#define USE_DEBUG
#define DEBUG_KEYWORDS "" 
#include "../std_lib_other.h"

int main() {
    std::cout << "Starting!" << std::endl;
    vicmil::TestClass::run_all_tests({".", "!L1_user"});
    std::cout << "Finished!" << std::endl;
}