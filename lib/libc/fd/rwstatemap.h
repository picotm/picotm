
#ifndef RWSTATEMAP_H
#define RWSTATEMAP_H

struct rwstatemap
{
    struct pgtreess super;
};

int
rwstatemap_init(struct rwstatemap *rwstatemap);

void
rwstatemap_uninit(struct rwstatemap *rwstatemap);

int
rwstatemap_rdlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap);

int
rwstatemap_wrlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap);

int
rwstatemap_unlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap);

/*int
rwstatemap_unlock_all(struct rwstatemap *rwstatemap,
                      struct rwlockmap *rwlockmap);*/

#endif

