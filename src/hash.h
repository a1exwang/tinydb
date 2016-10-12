#pragma once

typedef struct __Hash Hash;
typedef struct __HashKeyType {
  union {
    unsigned int n;
    void *p;
  };
} HashKeyType;

typedef unsigned int (*HashFn)(HashKeyType);
/**
 * Non-zero for a != b
 * Zero for a == b
 *
 */
typedef int (*HashKeyCmpFn)(HashKeyType a, HashKeyType b);

Hash *hashAlloc(HashFn pHashFn, HashKeyCmpFn pHashKeyCmpFn);
/**
 * Zero for success,
 * Non-zero for error.
 */
int hashPut(Hash *pHash, HashKeyType key, void *value);
int hashGet(Hash *pHash, HashKeyType key, void **value);
int hashRemove(Hash *pHash, HashKeyType key, void **value);
void hashFree(Hash *pHash);

int hashSize(Hash *pHash);

HashKeyType hashMakeKey(unsigned int n);

int hashIterFirst(Hash *pHash, HashKeyType *pKey, void **pValue);
int hashIterNext(Hash *pHash, HashKeyType current, HashKeyType *pKey, void **pValue);
void hashDebugPrintPage(Hash *pHash);
