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
  return hashMapGetNode(set, data) != NULL;
}

void *hashSetGetExisting(HashSet *set, void *data)
{
  HashMapNode *node = hashMapGetNode(set, data);
  return node != NULL ? node->key : NULL;
}

void hashSetRemove(HashSet *set, void *data)
{
  hashMapRemove(set, data);
}
