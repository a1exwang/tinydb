#include <hash.h>
#include <string.h>
#include <stdlib.h>
#include <tinydb.h>
#include <assert.h>
#include <stdio.h>
#include <pager.h>

#define HASH_DEFAULT_HASH_TABLE_SIZE 32

typedef struct __HashEntryType {
  struct __HashEntryType *pNext;
  struct __HashEntryType *pNextOfAll;
  struct __HashEntryType *pPrevOfAll;
  HashKeyType key;
  void *value;
} HashEntryType;

typedef struct __Hash {
  HashFn pHashFn;
  HashKeyCmpFn pHashKeyCmpFn;
  HashEntryType **aEntries;
  HashEntryType *pFirstEntry;
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
  pHash->pFirstEntry = 0;
  memset(pHash->aEntries, 0, sizeof(pHash->nCap * sizeof(HashEntryType)));
  return pHash;
}

static void hashEntryAppend(Hash *pHash, HashEntryType **head, HashKeyType key, void *value) {
  // empty list
  if (*head == 0) {
    *head = (HashEntryType *)malloc(sizeof(HashEntryType));
    (*head)->pNext = 0;
  }
  else {
    HashEntryType *prevHead = *head;
    *head = malloc(sizeof(HashEntryType));
    (*head)->pNext = prevHead;
  }
  (*head)->key = key;
  (*head)->value = value;

  if (pHash->pFirstEntry == 0) {
    pHash->pFirstEntry = *head;
    (*head)->pNextOfAll = *head;
    (*head)->pPrevOfAll = *head;
  }
  else {
    HashEntryType *pPrev = pHash->pFirstEntry->pPrevOfAll;
    pPrev->pNextOfAll = *head;
    pHash->pFirstEntry->pPrevOfAll = *head;
    (*head)->pNextOfAll = pHash->pFirstEntry;
    (*head)->pPrevOfAll = pPrev;
  }
}
static int hashEntryRemove(Hash *pHash, HashEntryType **head, HashKeyType key, void **pValue) {
  // empty list
  if (*head == 0) {
    return TINYDB_STATUS_NOT_FOUND;
  }
  else {
    HashEntryType *pPrev = 0;
    HashEntryType *pEntry = *head;
    while (pEntry != 0 && pHash->pHashKeyCmpFn(pEntry->key, key) != 0) {
      pPrev = pEntry;
      pEntry = pEntry->pNext;
    }
    if (pEntry == 0)
      return TINYDB_STATUS_NOT_FOUND;
    HashEntryType *next = pEntry->pNext;
    if (pPrev == 0)
      *head = next;
    else
      pPrev->pNext = next;
    if (pValue != 0)
      *pValue = pEntry->value;

    /* Update linked-list of all entries */
    if (pEntry == pHash->pFirstEntry) {
      pHash->pFirstEntry = pEntry->pNextOfAll;
    }
    pEntry->pPrevOfAll->pNextOfAll = pEntry->pNextOfAll;
    pEntry->pNextOfAll->pPrevOfAll = pEntry->pPrevOfAll;

    free(pEntry);
    return TINYDB_STATUS_OK;
  }
}
int hashEntryGet(Hash *pHash, HashKeyType key, HashEntryType **ppEntry) {
  unsigned int index = pHash->pHashFn(key) % pHash->nCap;
  *ppEntry = pHash->aEntries[index];

  // while entry.key != key
  while (*ppEntry != 0 && pHash->pHashKeyCmpFn((*ppEntry)->key, key) != 0) {
    *ppEntry = (*ppEntry)->pNext;
  }
  return *ppEntry == 0 ? TINYDB_STATUS_NOT_FOUND : TINYDB_STATUS_OK;
}

int hashGet(Hash *pHash, HashKeyType key, void **value) {
  HashEntryType *pEntry;
  int status = hashEntryGet(pHash, key, &pEntry);
  if (status == TINYDB_STATUS_OK && value != 0)
    *value = pEntry->value;
  return status;
}

int hashPut(Hash *pHash, HashKeyType key, void *value) {
  unsigned int index = pHash->pHashFn(key) % pHash->nCap;

  if (hashGet(pHash, key, 0) == 0) {
    return TINYDB_STATUS_DUPLICATE;
  }
  hashEntryAppend(pHash, &pHash->aEntries[index], key, value);
  pHash->nUsed++;
  return TINYDB_STATUS_OK;
}

int hashRemove(Hash *pHash, HashKeyType key, void **pValue) {
  unsigned int index = pHash->pHashFn(key) % pHash->nCap;

  if (hashGet(pHash, key, 0) != 0) {
    return TINYDB_STATUS_NOT_FOUND;
  }
  int status = hashEntryRemove(pHash, &pHash->aEntries[index], key, pValue);
  assert(status == TINYDB_STATUS_OK);
  pHash->nUsed--;
  return TINYDB_STATUS_OK;
}

void hashFree(Hash *pHash) {
  free(pHash);
}

int hashSize(Hash *pHash) {
  return pHash->nUsed;
}
/**
 * Get first entry of hash. Used to start iteration.
 * @param pHash: Hash object.
 * @param pKey: OUT, first key.
 * @param pValue: OUT, first value.
 * @return:
 *  TINYDB_STATUS_OK: Success.
 *  TINYDB_STATUS_NOT_FOUND: The hash table is empty.
 */
int hashIterFirst(Hash *pHash, HashKeyType *pKey, void **pValue) {
  if (pHash->pFirstEntry == 0) {
    return TINYDB_STATUS_NOT_FOUND;
  }
  else {
    *pKey = pHash->pFirstEntry->key;
    *pValue = pHash->pFirstEntry->value;
    return pHash->pFirstEntry->pNextOfAll == pHash->pFirstEntry ? TINYDB_STATUS_OK :TINYDB_STATUS_CONTINUE;
  }
}
/**
 * Iterate all entries in hash.
 * NOTE: DO NOT put/delete entries while iterating.
 * @param pHash: Hash object.
 * @param current: Current hash key.
 * @param pKey: OUT, next hash key.
 * @param pValue: OUT, next hash value.
 * @return:
 *  TINYDB_STATUS_OK: Iteration done, pKey and pValue are undefined.
 *  TINYDB_STATUS_CONTINUE: Iteration not done, pKey and pValue are next hash element.
 *  TINYDB_STATUS_PARAM_ERR: `current` key not found.
 */
int hashIterNext(Hash *pHash, HashKeyType current, HashKeyType *pKey, void **pValue) {
  HashEntryType *pEntryCurrent;
  int status = hashEntryGet(pHash, current, &pEntryCurrent);
  if (status != TINYDB_STATUS_OK)
    return TINYDB_STATUS_PARAM_ERR;

  HashEntryType *pEntryNext = pEntryCurrent->pNextOfAll;
  assert(pEntryNext != 0 && pEntryNext != pHash->pFirstEntry);
  *pKey = pEntryCurrent->pNextOfAll->key;
  *pValue = pEntryCurrent->pNextOfAll->value;
  return (pEntryNext->pNextOfAll == pHash->pFirstEntry) ? TINYDB_STATUS_OK : TINYDB_STATUS_CONTINUE;
}

void hashDebugPrintPage(Hash *pHash) {
  printf("hash<0x%p>:\n\tUsed = %d\n", pHash, pHash->nUsed);
  if (pHash->nUsed != 0) {
    HashKeyType key;
    void *pPage;
    int status = hashIterFirst(pHash, &key, (void**)&pPage);
    printf("(0x%08x, %p)\n", key.n, pPage);
    while (status == TINYDB_STATUS_CONTINUE) {
      status = hashIterNext(pHash, key, &key, (void **) &pPage);
      printf("(0x%08x, %p)\n", key.n, pPage);
    }
  }
}
