#include "Base_ArrayList.h"
#include "TestRegistry.h"

#include <stdio.h>

START_TEST(Append_test)
{
    ArrayList_t* list = ArrayList(size_t, 16);
    for(size_t i = 0; i < 100; i++)
    {
        int s = ArrayList_Append(list, &i);
        ck_assert(s == 0);
    }

    ck_assert(list->count == 100);

    for(size_t i = 0; i < 100; i++)
    {
        ck_assert(ArrayList_GetAs(size_t, list, i) == i);
    }

    ArrayList_Destroy(list);
}

START_TEST(AppendRange_test)
{
    ArrayList_t* list = ArrayList(size_t, 16);
    for(size_t i = 0; i < 10; i++)
    {
        size_t range[10];
        for(size_t j = 0; j < 10; j++)
        {
            range[j] = i * 10 + j;
        }

        int s = ArrayList_AppendRange(list, range, 10);
        ck_assert(s == 0);
    }

    ck_assert(list->count == 100);

    for(size_t i = 0; i < 100; i++)
    {
        ck_assert(ArrayList_GetAs(size_t, list, i) == i);
    }

    ArrayList_Destroy(list);
}


START_TEST(Insert_test)
{
    ArrayList_t* list = ArrayList(size_t, 16);
    for(size_t i = 0; i < 100; i++)
    {
        int s = ArrayList_Insert(list, 0, &i);
        ck_assert(s == 0);

        printf("Inserted %zu\n", i);
        fflush(stdout);
    }

    ck_assert(list->count == 100);

    for(size_t i = 0; i < 100; i++)
    {
        ck_assert(ArrayList_GetAs(size_t, list, i) == 99 - i);
    }

    ArrayList_Destroy(list);
}

START_TEST(InsertRange_test)
{
    ArrayList_t* list = ArrayList(size_t, 16);
    for(size_t i = 0; i < 10; i++)
    {
        size_t range[10];
        for(size_t j = 0; j < 10; j++)
        {
            range[j] = i * 10 + j;
        }

        int s = ArrayList_InsertRange(list, 0, range, 10);
        ck_assert(s == 0);
    }

    ck_assert(list->count == 100);

    for(size_t i = 0; i < 10; i++)
    {
        for(size_t j = 0; j < 10; j++)
        {
            printf("Checking %zu %zu\r\n", 99 - (i * 10) - 9 + j, ArrayList_GetAs(size_t, list, i * 10 + j));
            ck_assert(ArrayList_GetAs(size_t, list, i * 10 + j) == (99 - (i * 10) - 9 + j));
        }
    }

    ArrayList_Destroy(list);
}

START_TEST(Reserve_test)
{
    ArrayList_t* list = ArrayList(size_t, 16);
    int s             = ArrayList_Reserve(list, 100);
    ck_assert(s == 0);
    ck_assert(list->capacity == 100);

    ArrayList_Destroy(list);
}

START_TEST(Expand_test)
{
    ArrayList_t* list = ArrayList(size_t, 16);
    int s             = ArrayList_Realloc(list, 100);
    ck_assert(s == 0);
    ck_assert(list->capacity == 116);

    ArrayList_Destroy(list);
}

START_TEST(Remove_test)
{
    ArrayList_t* list = ArrayList(size_t, 16);
    for(size_t i = 0; i < 100; i++)
    {
        int s = ArrayList_Append(list, &i);
        ck_assert(s == 0);
    }

    ck_assert(list->count == 100);

    ArrayList_Remove(list, 0);
    ArrayList_Remove(list, 10 - 1);
    ArrayList_Remove(list, 20 - 2);
    ArrayList_Remove(list, 30 - 3);
    ArrayList_Remove(list, 40 - 4);

    ck_assert(list->count == 95);

    for(size_t i = 0; i < 95; i++)
    {
        size_t expected_value = i;
        if(i >= 0)
            expected_value++;
        if(i >= 9)
            expected_value++;
        if(i >= 18)
            expected_value++;
        if(i >= 27)
            expected_value++;
        if(i >= 36)
            expected_value++;

        ck_assert(ArrayList_GetAs(size_t, list, i) == expected_value);
    }

    ArrayList_Destroy(list);
}

__attribute__((constructor)) void register_carraylist_suite(void)
{
    TR_AddTest("ArrayList", "Core", Append_test);
    TR_AddTest("ArrayList", "Core", AppendRange_test);
    TR_AddTest("ArrayList", "Core", Insert_test);
    TR_AddTest("ArrayList", "Core", InsertRange_test);
    TR_AddTest("ArrayList", "Core", Reserve_test);
    TR_AddTest("ArrayList", "Core", Expand_test);
    TR_AddTest("ArrayList", "Core", Remove_test);
}
