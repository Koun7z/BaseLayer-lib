#include "TestRegistry.h"


#include "Base_HashMap.h"
#include "Base_String.h"

#include <stdio.h>

static HashMap_t* suite_registry = NULL;
static HashMap_t* tcase_registry = NULL;

void tr_add_test(const char* tsuite, const char* tcase, const TTest* tf)
{
    if(suite_registry == NULL)
    {
        suite_registry = StringMap(Suite*, 16);
    }

    if(tcase_registry == NULL)
    {
        tcase_registry = StringMap(TCase*, 16);
    }

    HashMapEntry* s_entry = StringMap_Find(suite_registry, tsuite);
    Suite* s              = NULL;
    if(s_entry == NULL)
    {
        s = suite_create(tsuite);

        StringMap_Insert(suite_registry, tsuite, s, sizeof(Suite*));
    }
    else
    {
        s = HashMapEntry_GetValue(s_entry);
    }

    HashMapEntry* t_entry = StringMap_Find(tcase_registry, tcase);
    TCase* tc             = NULL;
    if(t_entry == NULL)
    {
        tc = tcase_create(tcase);
        StringMap_Insert(tcase_registry, tcase, tc, sizeof(TCase*));
        suite_add_tcase(s, tc);
    }
    else
    {
        tc = HashMapEntry_GetValue(t_entry);
    }

    tcase_add_test(tc, tf);
}

Suite** tr_get_registered_suites(void)
{
    size_t count   = tr_get_registered_suite_count();
    Suite** suites = (Suite**) malloc(sizeof(Suite*) * count);

    size_t i        = 0;
    HashMapEntry* e = suite_registry->head_entry;
    while(e)
    {
        Suite** s   = HashMapEntry_GetValue(e);
        suites[i++] = *s;
        e           = e->next_iter;
    }
    return suites;
}

size_t tr_get_registered_suite_count(void)
{
    return suite_registry ? suite_registry->count : 0;
}