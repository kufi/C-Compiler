#include <stdlib.h>
#include <stdbool.h>
#include "HashMap.h"

/**
 * Simple Bob Jenkins's hash algorithm taken from the
 * wikipedia description.
 */
static uint32_t defaultHash(void *a)
{
    char *key = (char *)a;
    size_t len = strlen(key);
    uint32_t hash = 0;
    uint32_t i = 0;

    for(hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

static bool defaultCompare(void *a, void *b)
{
    return strcmp((char *)a, (char *)b) == 0;
}


HashMap *hashMapCreate()
{
  return hashMapCreateFull(0, 0.0f, NULL, NULL);
}

HashMap *hashMapCreateCap(int capacity)
{
  return hashMapCreateFull(capacity, 0.0f, NULL, NULL);
}

HashMap *hashMapCreateFull(int capacity, float loadFactor, HashMapCompare compareFunction, HashMapHash hashFunction)
{
  capacity = capacity <= 0 ? 16 : capacity;
  loadFactor = loadFactor <= 0.0f ? 0.75f : loadFactor;

  HashMap *map = calloc(1, sizeof(HashMap));
  map->capacity = capacity;
  map->loadFactor = loadFactor;
  map->compareFunction = compareFunction == NULL ? defaultCompare : compareFunction;
  map->hashFunction = hashFunction == NULL ? defaultHash : hashFunction;
  map->buckets = arrayListCreate(capacity, sizeof(LinkedList *));
  return map;
}

static LinkedList *hashMapGetBucket(HashMap *map, uint32_t hash, bool create)
{
  int bucketNumber = hash % map->capacity;
  LinkedList *bucket = arrayListGet(map->buckets, bucketNumber);

  if(bucket == NULL && create)
  {
    bucket = linkedListCreate();
    arrayListSet(map->buckets, bucketNumber, bucket);
  }

  return bucket;
}

static HashMapNode *hashMapCreateNode(uint32_t hash, void *key, void *data)
{
  HashMapNode *node = malloc(sizeof(HashMapNode));
  node->hash = hash;
  node->key = key;
  node->data = data;
  return node;
}

static void hashMapRehash(HashMap *map)
{
  float currentLoad = (float)map->count / map->capacity;
  if(currentLoad < map->loadFactor) return;

  ArrayList *oldBuckets = map->buckets;

  map->capacity *= 2;
  map->buckets = arrayListCreate(map->capacity, sizeof(LinkedList *));

  for(int i = 0; i < oldBuckets->max; i++)
  {
    LinkedList *oldList = arrayListGet(oldBuckets, i);
    if(oldList == NULL) continue;

    LIST_FOREACH(oldList, node)
    {
      HashMapNode *hashMapNode = node->item;
      linkedListPush(hashMapGetBucket(map, hashMapNode->hash, true), hashMapNode);
    }

    linkedListFree(oldList);
  }

  arrayListFree(oldBuckets);
}

void hashMapRemove(HashMap *map, void *key)
{
  uint32_t hash = map->hashFunction(key);

  LinkedList *bucket = hashMapGetBucket(map, hash, false);
  if(bucket == NULL) return;

  LIST_FOREACH(bucket, node)
  {
    HashMapNode *hashMapNode = node->item;
    if(hashMapNode->hash == hash && map->compareFunction(hashMapNode->key, key))
    {
      linkedListRemove(bucket, node);
    }
  }
}

void hashMapSet(HashMap *map, void *key, void *data)
{
  uint32_t hash = map->hashFunction(key);

  LinkedList *bucket = hashMapGetBucket(map, hash, true);

  LIST_FOREACH(bucket, node)
  {
    HashMapNode *hashMapNode = node->item;
    if(hashMapNode->hash == hash && map->compareFunction(hashMapNode->key, key))
    {
      hashMapNode->data = data;
      return;
    }
  }

  linkedListPush(bucket, hashMapCreateNode(hash, key, data));
  map->count++;

  hashMapRehash(map);
}

HashMapNode *hashMapGetNode(HashMap *map, void *key)
{
  uint32_t hash = map->hashFunction(key);

  LinkedList *bucket = hashMapGetBucket(map, hash, false);

  if(bucket == NULL) return NULL;

  LIST_FOREACH(bucket, node)
  {
    HashMapNode *hashMapNode = node->item;
    if(hashMapNode->hash == hash && map->compareFunction(hashMapNode->key, key))
    {
      return hashMapNode;
    }
  }

  return NULL;
}

void *hashMapGet(HashMap *map, void *key)
{
  HashMapNode *node = hashMapGetNode(map, key);
  return node != NULL ? node->data : NULL;
}
