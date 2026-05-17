#include <stdio.h>
#include <stdlib.h>

#include "TestRegistry.h"
#include "HashMap/HashMap_TestKeys.h"

key_table_t test_keys;

int main()
{
    test_keys = key_table_create(1000, 16, 42);

    Suite** suites     = TR_GetRegisteredSuites();
    size_t suite_count = TR_GetRegisteredSuiteCount();

    SRunner* runner = srunner_create(suites[0]);
    for(size_t i = 1; i < suite_count; i++)
    {
        srunner_add_suite(runner, suites[i]);
    }

    srunner_run_all(runner, CK_VERBOSE);
    int number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    free(suites);
    free(test_keys.data);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}