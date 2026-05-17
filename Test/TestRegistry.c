#include "TestRegistry.h"


#include "Base_HashMap.h"
#include "Base_String.h"

// Yeah lets the very module that this is supposed to test,  what could go wrong? :D

static HashMap_t* suite_registry = NULL;
static HashMap_t* tcase_registry = NULL;

void TR_AddTest(const char* tsuite, const char* tcase, const TTest* tf)
{
    if(suite_registry == NULL)
    {
        suite_registry = StringMap(Suite*, 16);
    }

    if(tcase_registry == NULL)
    {
        tcase_registry = StringMap(TCase*, 16);
    }

    HashMapEntry* s_entry = CStringMap_Find(suite_registry, tsuite);
    Suite* s              = NULL;
    if(s_entry == NULL)
    {
        s = suite_create(tsuite);

        CStringMap_InsertPointer(suite_registry, tsuite, s);
    }
    else
    {
        s = *HashMapEntry_GetValueAs(Suite*, s_entry);
    }

    HashMapEntry* t_entry = CStringMap_Find(tcase_registry, tcase);
    TCase* tc             = NULL;
    if(t_entry == NULL)
    {
        tc = tcase_create(tcase);
        CStringMap_InsertPointer(tcase_registry, tcase, tc);
        suite_add_tcase(s, tc);
    }
    else
    {
        tc = *HashMapEntry_GetValueAs(TCase*, t_entry);
    }

    tcase_add_test(tc, tf);
}

Suite** TR_GetRegisteredSuites(void)
{
    size_t count   = TR_GetRegisteredSuiteCount();
    Suite** suites = (Suite**) malloc(sizeof(Suite*) * count);

    size_t i        = 0;
    HashMapEntry* e = suite_registry->head_entry;
    while(e)
    {
        suites[i++] = *HashMapEntry_GetValueAs(Suite*, e);
        e           = e->next_iter;
    }
    return suites;
}

size_t TR_GetRegisteredSuiteCount(void)
{
    return suite_registry ? suite_registry->count : 0;
}