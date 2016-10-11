#include <tinydb.h>
#include <pager.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

/**
 * typedef struct __Page {...} Page;
 * Modify the page struct only with pagerPageXXX
 */
typedef struct __Page {
  unsigned    id;
  int         iFlag;
  int         bDirty;
  int         iRefCount;
  char        aBuf[PAGER_PAGE_SIZE];
} Page;

inline static void *pagerPageGetBuf(void *pPage) {
  return ((Page*)pPage)->aBuf;
}
inline static Page *pagerPageGetPageFromBuf(void *pBuf) {
  return (Page*)((char*)pBuf - ((unsigned int)((Page*)0)->aBuf));
}
inline static Page *pagerPageAlloc(unsigned int id) {
  Page *pPage = malloc(sizeof(Page));
  memset(pPage, 0, sizeof(Page));
  pPage->id = id;
  pPage->iRefCount = 1;
  return pPage;
}
inline static int pagerPageRef(Page *pPage) {
  return pPage->iRefCount;
}
/**
 * return new ref count;
 */
inline static int pagerPageRefInc(Page *pPage) {
  pPage->iRefCount++;
  return pPage->iRefCount;
}
/**
 * return new ref count;
 */
inline static int pagerPageRefDec(Page *pPage) {
  assert(pPage->iRefCount > 0);
  if (pPage->iRefCount == 1) {
    free(pPage);
    return 0;
  }
  else {
    pPage->iRefCount--;
    return pPage->iRefCount;
  }
}

Pager *pagerOpen(const char *filePath, PagerOpenFlag flag) {
  Pager *pager = (Pager*) malloc(sizeof(Pager));
  pager->zFilePath = malloc(strlen(filePath));
  strcpy(pager->zFilePath, filePath);

  int oFlag = O_CREAT;
  if (flag == PAGER_OPEN_READ_ONLY)
    oFlag |= O_RDONLY;
  else if (flag == PAGER_OPEN_READ_WRITE)
    oFlag |= O_RDWR;
  else {
    fprintf(stderr, "wrong argument: flag\n");
    exit(1);
  }

  pager->iFd = open(pager->zFilePath, oFlag, S_IRWXU);
  if (pager->iFd == -1) {
    fprintf(stderr, "open() error: %s\n", strerror(errno));
    exit(1);
  }

  // init page hash table
  pager->pHash = hashAlloc(0, 0);
  return pager;
}

void pagerClose(Pager *pPager) {
  free(pPager->zFilePath);
  hashFree(pPager->pHash);
  free(pPager);
}


void *pagerGetPage(Pager *pPager, unsigned int id) {
  int status;

  Page *pPage = 0;
  if (hashGet(pPager->pHash, hashMakeKey(id), (void**)&pPage) == 0) {
    pagerPageRefInc(pPage);
    return pagerPageGetBuf(pPage);
  }

  /**
   * Read page `id` from file to `data`.
   */
  lseek(pPager->iFd, 0, SEEK_END);
  long fsize = lseek(pPager->iFd, 0, SEEK_CUR);
  // file size must be integer times of PAGER_PAGE_SIZE
  assert(fsize % PAGER_PAGE_SIZE == 0);
  if (fsize < (id+1) * PAGER_PAGE_SIZE) {
    status = ftruncate(pPager->iFd, (id+1) * PAGER_PAGE_SIZE);
    if (status != 0) {
      fprintf(stderr, "ftruncate error: %s\n", strerror(errno));
      exit(1);
    }
  }

  lseek(pPager->iFd, id * PAGER_PAGE_SIZE, SEEK_SET);

  pPage = pagerPageAlloc(id);
  ssize_t actualSize = read(pPager->iFd, pagerPageGetBuf(pPage), PAGER_PAGE_SIZE);
  assert(actualSize == PAGER_PAGE_SIZE);
  /**
   * Read done.
   */

  // Put it in hash

  assert(hashPut(pPager->pHash, hashMakeKey(id), pPage) == 0);

  return pagerPageGetBuf(pPage);
}

int pagerMarkDirty(Pager *pager, void *pBuf) {
  Page *pPage = pagerPageGetPageFromBuf(pBuf);
  pPage->bDirty = 1;
  return TINYDB_STATUS_OK;
}

int pagerReleasePage(Pager *pager, void *pBuf) {
  int status;
  Page *pPage = pagerPageGetPageFromBuf(pBuf);
  int nCount = pagerPageRef(pPage);
  if (nCount == 1) {
    if (pPage->bDirty) {
      lseek(pager->iFd, pPage->id * PAGER_PAGE_SIZE, SEEK_SET);
      assert(write(pager->iFd, pPage->aBuf, PAGER_PAGE_SIZE) == PAGER_PAGE_SIZE);
    }
    status = hashRemove(pager->pHash, hashMakeKey(pPage->id), 0);
    assert(status == 0);
  }
  pagerPageRefDec(pPage);
  return TINYDB_STATUS_OK;
}
