#pragma once

#include "L1_debug.h"
#include <string>
#include <map>

namespace vicmil {
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
        test_name() : vicmil::TestClass(GetLineIdentifier, GetLineIdentifierLong) {} \
        func \
    }; \
} \
namespace test_factory { \
    test_class::test_name test_name = test_class::test_name(); \
}
#else
#define TestWrapper(test_name, func)
#endif

}