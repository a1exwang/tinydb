#include <tinydb.h>
#include <pager.h>
#include <test_utils.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

int main() {
  testUtilsInit();

  int status;
  const char *tmpFilePath = testUtilsGetTempFilePath();
  Pager *pager = pagerOpen(tmpFilePath, PAGER_OPEN_READ_WRITE);
  assert(pager != 0);

  char *page0 = pagerGetPage(pager, 0);
  char *page15 = pagerGetPage(pager, 15);

  // check newly created pages are filled with zero
  for (int i = 0; i < PAGER_PAGE_SIZE; ++i) {
    assert(page0[i] == 0);
  }
  for (int i = 0; i < PAGER_PAGE_SIZE; ++i) {
    assert(page15[i] == 0);
  }

  page0[0] = 'A';
  page15[0] = 'A';
  status = pagerMarkDirty(pager, page15);
  assert(status == TINYDB_STATUS_OK);

  status = pagerReleasePage(pager, page0);
  assert(status == TINYDB_STATUS_OK);
  status = pagerReleasePage(pager, page15);
  assert(status == TINYDB_STATUS_OK);

  pagerClose(pager);
  pager = 0;

  int fd = open(tmpFilePath, O_RDONLY);
  assert(fd > 0);
  char *buffer = malloc(PAGER_PAGE_SIZE * 16);
  ssize_t actuallyRead = read(fd, buffer, PAGER_PAGE_SIZE * 16);
  assert(actuallyRead == PAGER_PAGE_SIZE * 16);
  assert(buffer[0] == 0);
  assert(buffer[PAGER_PAGE_SIZE * 15] == 'A');

  free(buffer);
  close(fd);

  remove(tmpFilePath);
  testUtilsMemFree(tmpFilePath);

  printf("pager test done, no error.\n");
  return 0;
}
