#include <hash.h>
#include <string.h>
#include <stdlib.h>

#define HASH_DEFAULT_HASH_TABLE_SIZE 32

typedef struct __HashEntryType {
  struct __HashEntryType *pNext;
  HashKeyType key;
  void *value;
} HashEntryType;

typedef struct __Hash {
  HashFn pHashFn;
  HashKeyCmpFn pHashKeyCmpFn;
  HashEntryType **aEntries;
  int nCap;
  int nUsed;
} Hash;

static unsigned int __defaultHashFn(HashKeyType key) {
  return key.n;
}
static int __defaultHashKeyCmpFn(HashKeyType a, HashKeyType b) {
  return a.n - b.n;
}

HashKeyType hashMakeKey(unsigned int n) {
  HashKeyType ret;
  ret.n = n;
  return ret;
}
Hash *hashAlloc(HashFn pHashFn, HashKeyCmpFn pHashKeyCmpFn) {
  Hash *pHash = malloc(sizeof(Hash));
  memset(pHash, 0, sizeof(Hash));
  pHash->pHashFn = pHashFn == 0 ? __defaultHashFn : pHashFn;
  pHash->pHashKeyCmpFn = pHashKeyCmpFn == 0 ? __defaultHashKeyCmpFn : pHashKeyCmpFn;
  pHash->nCap = HASH_DEFAULT_HASH_TABLE_SIZE;
  pHash->nUsed = 0;
  pHash->aEntries = malloc(pHash->nCap * sizeof(HashEntryType*));
  memset(pHash->aEntries, 0, sizeof(pHash->nCap * sizeof(HashEntryType)));
  return pHash;
}

static void hashEntryAppend(HashEntryType **head, HashKeyType key, void *value) {
  // empty list
  if (*head == 0) {
    *head = (HashEntryType *)malloc(sizeof(HashEntryType));
    (*head)->pNext = 0;
    (*head)->key = key;
    (*head)->value = value;
  }
  else {
    HashEntryType *prevHead = *head;
    *head = malloc(sizeof(HashEntryType));
    (*head)->pNext = prevHead;
    (*head)->key = key;
    (*head)->value = value;
  }
}
static int hashEntryRemove(Hash *pHash, HashEntryType **head, HashKeyType key, void **pValue) {
  // empty list
  if (*head == 0) {
    return -1;
  }
  else {
    HashEntryType *pPrev = 0;
    HashEntryType *pEntry = *head;
    while (pEntry != 0 && pHash->pHashKeyCmpFn(pEntry->key, key) != 0) {
      pPrev = pEntry;
      pEntry = pEntry->pNext;
    }
    if (pEntry == 0)
      return -1;
    HashEntryType *next = pEntry->pNext;
    if (pPrev == 0)
      *head = next;
    else
      pPrev->pNext = next;
    if (pValue != 0)
      *pValue = pEntry->value;
    return 0;
  }
}

int hashGet(Hash *pHash, HashKeyType key, void **value) {
  unsigned int index = pHash->pHashFn(key) % pHash->nCap;
  HashEntryType *pEntry = pHash->aEntries[index];

  // while entry.key != key
  while (pEntry != 0 && pHash->pHashKeyCmpFn(pEntry->key, key) != 0) {
    pEntry = pEntry->pNext;
  }
  if (pEntry == 0)
    return 1;

  if (value != 0)
    *value = pEntry->value;
  return 0;
}

int hashPut(Hash *pHash, HashKeyType key, void *value) {
  unsigned int index = pHash->pHashFn(key) % pHash->nCap;

  if (hashGet(pHash, key, 0) == 0) {
    return 1;
  }
  hashEntryAppend(&pHash->aEntries[index], key, value);
  pHash->nUsed++;
  return 0;
}

int hashRemove(Hash *pHash, HashKeyType key, void **pValue) {
  unsigned int index = pHash->pHashFn(key) % pHash->nCap;

  if (hashGet(pHash, key, 0) != 0) {
    return 1;
  }
  hashEntryRemove(pHash, &pHash->aEntries[index], key, pValue);
  pHash->nUsed--;
  return 0;
}

void hashFree(Hash *pHash) {
  free(pHash);
}
