#include <test_utils.h>
#include <assert.h>
#include <hash.h>
#include <stdio.h>
#include <tinydb.h>

int main() {
  testUtilsInit();

  Hash *hash = hashAlloc(0, 0);

  void *value;
  assert(hashGet(hash, hashMakeKey(0), &value) != TINYDB_STATUS_OK);

  value = (void*) 1;
  assert(hashPut(hash, hashMakeKey(10), value) == TINYDB_STATUS_OK);
  assert(hashGet(hash, hashMakeKey(10), &value) == TINYDB_STATUS_OK);
  assert(value == (void*)1);

  // put(10, 1), should be error duplicate
  value = (void*) 1;
  assert(hashPut(hash, hashMakeKey(10), value) == TINYDB_STATUS_DUPLICATE);

  // put(10 + 32, 2), should be ok
  value = (void*) 2;
  assert(hashPut(hash, hashMakeKey(10 + 32), value) == TINYDB_STATUS_OK);

  // get(10 + 32) == 2, should be ok
  assert(hashGet(hash, hashMakeKey(10 + 32), &value) == TINYDB_STATUS_OK);
  assert(value == (void*) 2);

  // get(10) == 1, should be ok
  assert(hashGet(hash, hashMakeKey(10), &value) ==  TINYDB_STATUS_OK);
  assert(value == (void*) 1);

  assert(hashRemove(hash, hashMakeKey(10 + 32), &value) == TINYDB_STATUS_OK);
  assert(value == (void*) 2);

  assert(hashGet(hash, hashMakeKey(10 + 32), &value) == TINYDB_STATUS_NOT_FOUND);

  /**
   * Test iteration
   */
  HashKeyType key;
  value = (void*) 0;
  assert(hashPut(hash, hashMakeKey(0), value) == TINYDB_STATUS_OK);
  value = (void*) 1;
  assert(hashPut(hash, hashMakeKey(1), value) == TINYDB_STATUS_OK);

  assert(hashIterFirst(hash, &key, &value) == TINYDB_STATUS_CONTINUE);
  assert((key.n == 0 && value == (void*)0) || ((key.n == 1) && (void*)1 == value) || ((key.n == 10) && (void*)1 == value));
  assert(hashIterNext(hash, key, &key, &value) == TINYDB_STATUS_CONTINUE);
  assert((key.n == 0 && value == (void*)0) || ((key.n == 1) && (void*)1 == value) || ((key.n == 10) && (void*)1 == value));
  assert(hashIterNext(hash, key, &key, &value) == TINYDB_STATUS_OK);
  assert((key.n == 0 && value == (void*)0) || ((key.n == 1) && (void*)1 == value) || ((key.n == 10) && (void*)1 == value));

  printf("all test done\nno error");

  return 0;
}