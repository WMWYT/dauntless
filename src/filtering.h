#ifndef FILTERING_H
#define FILTERING_H

#include "uthash.h"
#include "utarray.h"

typedef struct{
    int max_qos;
    char * client_id;
}ChilderNode;

struct TrieNode{
    char * key;
    struct TrieNode * children;
    struct TrieNode * plus_children;

    struct{
        UT_array * childer_node;
    }PoundNode;

    UT_array * childer_node;

    UT_hash_handle hh;
};

struct RootNode{
    struct TrieNode * children;
    struct TrieNode * plus_children;
    UT_array * childer_node;
};

struct SYSNode{
    struct TrieNode * children;
    struct TrieNode * plus_children;
};

int intsort(const void *a, const void *b);
int strsort(const void *_a, const void *_b);

void intercept(char * key, int max_qos, char * client_id);
UT_array * search(char * key);
void delete_topic(char * key, char * client_id);
void delete_all(struct TrieNode * node);
void printf_all(struct TrieNode * s_root);

#endif