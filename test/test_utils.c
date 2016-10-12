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
#include <signal.h>
#include <execinfo.h>
#include <stdarg.h>

static int s_testCount = 0;

static void testUtilsSignalHandler(int sig) {
  void *array[10];
  int size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

void testUtilsInit(void) {
  signal(SIGSEGV, testUtilsSignalHandler);
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

void testUtilsAssertEqI(int a, int b) {
  if (a != b) {
    exit(1);
  }
  s_testCount++;
}

void testUtilsAssertEqIM(int a, int b, const char *msg, ...) {
  if (a != b) {
    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    exit(1);
  }
  s_testCount++;
}

void testUtilsEnd() {
  printf("Test finished, %d assertions. All clear!\n", s_testCount);
}
