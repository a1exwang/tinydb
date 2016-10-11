#include <test_utils.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void testUtilsInit(void) {
  srand((unsigned int)time(0));
}

const char *testUtilsGetTempFilePath(void) {
  int status;
  const char *dir = "/tmp/testDbTemp1234";
  struct stat sb;
  if (stat(dir, &sb) == 0) {
    if(!S_ISDIR(sb.st_mode)) {
      fprintf(stderr, "cannot mkdir /tmp/testDbTemp123, reason file exists\n");
      exit(1);
    }
  }
  else {
    status = mkdir(dir, 0755);
    if (status < 0 && status != EEXIST) {
      fprintf(stderr, "cannot make temp director of /tmp/testDbTemp1234\n");
      exit(1);
    }
  }

  // 32 bit integer is at most 8 hex long, so 20 byte should be enough.
  size_t len = strlen(dir) + 20;
  char *ret = malloc(len);
  sprintf(ret, "%s/file%08x", dir, (unsigned int)rand());

  return ret;
}

void testUtilsMemFree(const void *ptr) {
  free((void*)ptr);
}
