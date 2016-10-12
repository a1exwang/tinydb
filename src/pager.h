#pragma once
#include <hash.h>

#define PAGER_PAGE_SIZE   1024  // Pager size is 1KiB

typedef struct __Pager {
  char*       zFilePath;
  int         iFd;
  Hash*       pHash;
} Pager;

/**
 * Flags for pagerOpen()
 */
typedef int PagerOpenFlag;
#define PAGER_OPEN_READ_ONLY    1
#define PAGER_OPEN_READ_WRITE   2

Pager *pagerOpen(const char *filePath, PagerOpenFlag flags);

void *pagerGetPage(Pager *pPager, unsigned int id);
int pagerMarkDirty(Pager *pager, void *pBuf);
int pagerReleasePage(Pager *pager, void *pBuf);

void pagerClose(Pager *pPager);

