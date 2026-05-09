#include "TestRegistry.h"

#include "Base_HashMap.h"

START_TEST(Insertion_test)
{
    typedef struct
    {
        int a;
        float b;
    } test_struct_t;

    HashMap_t* map = HashMap(char, test_struct_t, 16);

    char* keys[]           = {"never", "gonna", "give", "you", "up"};
    test_struct_t values[] = {
        {.a = 1, .b = 1.0f},
        {.a = 2, .b = 2.0f},
        {.a = 3, .b = 3.0f},
        {.a = 4, .b = 4.0f},
        {.a = 5, .b = 5.0f},
    };

    for(size_t i = 0; i < 5; i++)
    {
        HashMap_InsertElement(map, keys[i], strlen(keys[i]) + 1, &values[i], sizeof(test_struct_t));
    }

    ck_assert_msg(0, "test");
}

__attribute__((constructor)) void register_hashmap_suite(void)
{
    tr_add_test("HashMap", "Core", Insertion_test);
}