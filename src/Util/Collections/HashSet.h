#ifndef HASHSET_HEADER
#define HASHSET_HEADER

#include <stdbool.h>
#include "HashMap.h"

typedef HashMap HashSet;

#define hashSetFor(M, V) hashMapFor(M, V)

#define hashSetForEnd hashMapForEnd

static inline void *hashSetForItem(LinkedListNode *node)
{
  return ((HashMapNode *)node->item)->key;
}

HashSet *hashSetCreate();

HashSet *hashSetCreateCap(int capacity);

HashSet *hashSetCreateFull(int capacity, float loadFactor, HashMapCompare compareFunction, HashMapHash hashFunction);

void hashSetPut(HashSet *set, void *data);

bool hashSetExists(HashSet *set, void *data);

void hashSetRemove(HashSet *set, void *data);

#endif
