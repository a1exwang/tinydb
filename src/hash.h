#pragma once

typedef struct __Hash Hash;
typedef struct __HashKeyType HashKeyType;

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

HashKeyType hashMakeKey(unsigned int n);

typedef struct __HashKeyType {
  union {
    unsigned int n;
    void *p;
  };
} HashKeyType;
