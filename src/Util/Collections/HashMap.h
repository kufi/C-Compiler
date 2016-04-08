#ifndef HASHMAP_HEADER
#define HASHMAP_HEADER

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "LinkedList.h"
#include "ArrayList.h"

typedef bool (*HashMapCompare)(void *a, void *b);
typedef uint32_t (*HashMapHash)(void *key);

typedef struct HashMap {
  HashMapCompare compareFunction;
  HashMapHash hashFunction;
  float loadFactor;
  int count;
  int capacity;
  ArrayList *buckets;
} HashMap;

typedef struct HashMapNode {
  uint32_t hash;
  void *key;
  void *data;
} HashMapNode;

#define hashMapFor(M, V) for(int _hashmap_i = 0; _hashmap_i < M->capacity; _hashmap_i++ ) {\
  LinkedList *_hashmap_bucket_list = M->buckets->items[_hashmap_i]; \
  if(_hashmap_bucket_list == NULL) continue; \
  LIST_FOREACH(_hashmap_bucket_list, V)

#define hashMapForEnd }

static inline void *hashMapForItem(LinkedListNode *node)
{
  return ((HashMapNode *)node->item)->data;
}

static inline void *hashMapForKey(LinkedListNode *node)
{
  return ((HashMapNode *)node->item)->key;
}

HashMap *hashMapCreate();

HashMap *hashMapCreateCap(int capacity);

HashMap *hashMapCreateFull(int capacity, float loadFactor, HashMapCompare compareFunction, HashMapHash hashFunction);

void hashMapSet(HashMap *map, void *key, void *data);

void *hashMapGet(HashMap *map, void *key);

HashMapNode *hashMapGetNode(HashMap *map, void *key);

void hashMapRemove(HashMap *map, void *key);

#endif
