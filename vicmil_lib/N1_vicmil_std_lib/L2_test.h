#pragma once

#include "L1_debug.h"
#include <string>
#include <map>
#include <set>

namespace vicmil {
/**
 * Type of a void function
*/
typedef void (*void_function_type)();


#ifdef TEST_KEYWORDS
    const std::string __test_keywords_raw__ = TEST_KEYWORDS;
#else
    const std::string __test_keywords_raw__ = "";
#endif

const std::vector<std::string> __test_keywords__ = split_string(__test_keywords_raw__, ',');





struct FactoryBase {
    std::string _id_long;
    virtual ~FactoryBase() {}
    virtual void test() {}
};

typedef std::map<std::string, FactoryBase*> FactoryMap;
static FactoryMap* factory_map = nullptr;

struct TestClass : public FactoryBase {
    TestClass(std::string id, std::string id_long) {
        if ( !factory_map ) {
            factory_map = new FactoryMap();
        }
        (*factory_map)[id] = this;
        _id_long = id_long;
    }
    static void run_all_tests(std::vector<std::string> test_keywords = __test_keywords__) {
        START_TRACE_FUNCTION();
        if(!factory_map) {
            Debug("No tests detected!");
            return;
        }
        FactoryMap::iterator it = factory_map->begin();
        Debug("start loop");
        while(it != factory_map->end()) {
            std::pair<const std::string, FactoryBase *> val = *it;
            it++;
            std::string test_name = val.first;
            if(should_run_test(val.second->_id_long, test_keywords)) {
                std::cout << "<<<<<<< run test: " << test_name << ">>>>>>>" << std::endl;
                val.second->test();
                std::cout << "test passed!" << std::endl;
            }
        }
        std::cout << "All tests passed!" << std::endl;;
    }
    static bool should_run_test(std::string test_name, std::vector<std::string>& test_keywords) {
        return match_keywords(test_name, test_keywords);
    }
};

#ifdef USE_DEBUG
#define TestWrapper(test_name, func) \
namespace test_class { \
    struct test_name : vicmil::TestClass { \
        test_name() : vicmil::TestClass(GetLineInfo, GetLongLineInfo) {} \
        func \
    }; \
} \
namespace test_factory { \
    test_class::test_name test_name = test_class::test_name(); \
}
#else
#define TestWrapper(test_name, func)
#endif

#define AddTest(test_name) \
TestWrapper(test_name ## _, \
void test() { \
    test_name(); \
} \
);

/*struct TestInstance {
    void_function_type func_ref;
    std::string identifier = "";
    std::string identifier_long = "";
    TestInstance(void_function_type func_ref_, std::string identifier_, std::string identifier_long_) {
        func_ref = func_ref_;
        identifier = identifier_;
        identifier_long = identifier_long_;
    }
};
class TestTracker {
    static std::set<TestInstance> tests;
public:
    static TestInstance _add_test_instance(TestInstance test_instance) {
        tests.insert(test_instance);
    }
    void run_tests(std::vector<std::string>& test_keywords) {

    }
}*/
}