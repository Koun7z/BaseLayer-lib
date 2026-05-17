#ifndef TEST_REGISTRY_H__
#define TEST_REGISTRY_H__

#include <check.h>

#if defined(__cplusplus)
extern "C" {
#endif

void TR_AddTest(const char* tsuite, const char* tcase, const TTest* tf);
Suite** TR_GetRegisteredSuites(void);
size_t TR_GetRegisteredSuiteCount(void);

#if defined(__cplusplus)
}
#endif

#endif /* TEST_REGISTRY_H__ */