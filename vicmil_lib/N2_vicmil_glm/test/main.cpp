#define USE_DEBUG
#define DEBUG_KEYWORDS ".,!vicmil_std_lib" 
#include "../vicmil_glm.h"

int main() {
    std::cout << "Starting!" << std::endl;
    vicmil::TestClass::run_all_tests({"."});
    std::cout << "Finished!" << std::endl;
}