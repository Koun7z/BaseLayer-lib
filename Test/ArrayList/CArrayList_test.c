#include "Base_CArrayList.h"
#include "TestRegistry.h"

#include <stdio.h>

START_TEST(Append_test)
{
    size_t* list = CArrayList(size_t, 16);
    for(size_t i = 0; i < 100; i++)
    {
        int s = CArrayList_Append(&list, i);
        ck_assert(s == 0);
    }

    ck_assert(CArrayList_Count(list) == 100);

    for(size_t i = 0; i < 100; i++)
    {
        ck_assert(list[i] == i);
    }

    CArrayList_Free(&list);
}

START_TEST(AppendRange_test)
{
    size_t* list = CArrayList(size_t, 16);
    for(size_t i = 0; i < 10; i++)
    {
        size_t range[10];
        for(size_t j = 0; j < 10; j++)
        {
            range[j] = i * 10 + j;
        }

        int s = CArrayList_AppendMany(&list, range, 10);
        ck_assert(s == 0);
    }

    ck_assert(CArrayList_Count(list) == 100);

    for(size_t i = 0; i < 100; i++)
    {
        ck_assert(list[i] == i);
    }

    CArrayList_Free(&list);
}


START_TEST(Insert_test)
{
    size_t* list = CArrayList(size_t, 16);
    for(size_t i = 0; i < 100; i++)
    {
        int s = CArrayList_Insert(&list, 0, i);
        ck_assert(s == 0);

        printf("Inserted %zu\n", i);
        fflush(stdout);
    }

    ck_assert(CArrayList_Count(list) == 100);

    for(size_t i = 0; i < 100; i++)
    {
        ck_assert(list[i] == 99 - i);
    }

    CArrayList_Free(&list);
}

__attribute__((constructor)) void register_carraylist_suite(void)
{
    TR_AddTest("CArrayList", "Core", Append_test);
    TR_AddTest("CArrayList", "Core", AppendRange_test);
    TR_AddTest("CArrayList", "Core", Insert_test);
}
