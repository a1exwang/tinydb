#pragma once

void testUtilsInit(void);
const char *testUtilsGetTempFilePath(void);
void testUtilsMemFree(const void *);

void testUtilsAssertEqI(int a, int b);
void testUtilsAssertEqIM(int a, int b, const char *msg, ...);
void testUtilsEnd();
