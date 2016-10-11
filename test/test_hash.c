#include <test_utils.h>
#include <assert.h>
#include <hash.h>
#include <stdio.h>

int main() {
  testUtilsInit();

  Hash *hash = hashAlloc(0, 0);

  void *value;
  assert(hashGet(hash, hashMakeKey(0), &value) != 0);

  value = (void*) 1;
  assert(hashPut(hash, hashMakeKey(10), value) == 0);
  assert(hashGet(hash, hashMakeKey(10), &value) == 0);
  assert(value == (void*)1);

  // put(10, 1), should be ok
  value = (void*) 1;
  assert(hashPut(hash, hashMakeKey(10), value) == 1);

  // put(10 + 32, 2), should be ok
  value = (void*) 2;
  assert(hashPut(hash, hashMakeKey(10 + 32), value) == 0);

  // get(10 + 32) == 2, should be ok
  assert(hashGet(hash, hashMakeKey(10 + 32), &value) == 0);
  assert(value == (void*) 2);

  // get(10) == 1, should be ok
  assert(hashGet(hash, hashMakeKey(10), &value) == 0);
  assert(value == (void*) 1);

  assert(hashRemove(hash, hashMakeKey(10 + 32), &value) == 0);
  assert(value == (void*) 2);

  assert(hashGet(hash, hashMakeKey(10 + 32), &value) == 1);

  printf("all test done\nno error");

  return 0;
}