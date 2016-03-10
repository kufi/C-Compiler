#include "HashSet.h"

HashSet *hashSetCreate()
{
  return hashMapCreate();
}

HashSet *hashSetCreateCap(int capacity)
{
  return hashMapCreateCap(capacity);
}

HashSet *hashSetCreateFull(int capacity, float loadFactor, HashMapCompare compareFunction, HashMapHash hashFunction)
{
  return hashMapCreateFull(capacity, loadFactor, compareFunction, hashFunction);
}

void hashSetPut(HashSet *set, void *data)
{
  hashMapSet(set, data, NULL);
}

bool hashSetExists(HashSet *set, void *data)
{
  return hashMapGet(set, data) != NULL;
}

void hashSetRemove(HashSet *set, void *data)
{
  hashMapRemove(set, data);
}
