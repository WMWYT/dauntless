#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include "filtering.h"

struct RootNode root;
struct SYSNode sys;
UT_icd node_icd = {sizeof(struct TrieNode), NULL, NULL, NULL};

#define DELETE(A, B)                        \
    int flag = 0;                           \
    if((B)->children != NULL) flag = 1;     \
    if((B)->plus_children != NULL) flag =1; \
    if((B)->childer_node != NULL)              \
        if(utarray_front((B)->childer_node) != NULL) flag = 1;\
    if((B)->PoundNode.childer_node != NULL)    \
        if(utarray_front((B)->PoundNode.childer_node) != NULL) flag = 1;\
    if(flag == 0)                           \
        if(!strcmp((B)->key, "+"))          \
            HASH_DEL((A)->plus_children, B);\
        else                                \
            HASH_DEL((A)->children, B);

int intsort(const void *a, const void *b){
    int _a = *(const int *)a;
    int _b = *(const int *)b;
    return (_a < _b) ? -1 : (_a > _b);
}

int strsort(const void *_a, const void *_b){
    const char *a = *(const char* const *)_a;
    const char *b = *(const char* const *)_b;

    return strcmp(a,b);
}

int struct_str_sort(const void * _b, const void * _a){
    const ChilderNode *a = (const ChilderNode *) _a;
    const char *b = (const char *) _b;

    return strcmp(a->client_id, b);
}

int structsort(const void * _a, const void * _b){
    const ChilderNode * a = (const ChilderNode *) _a;
    const ChilderNode * b = (const ChilderNode *) _b;

    return strcmp(a->client_id, b->client_id);
}

void childer_node_copy(void *_dst, const void *_src) {
  ChilderNode *dst = (ChilderNode*)_dst, *src = (ChilderNode*)_src;
  dst->max_qos = src->max_qos;
  dst->client_id = src->client_id ? strdup(src->client_id) : NULL;
}

void childer_node_dtor(void *_elt) {
  ChilderNode *elt = (ChilderNode*)_elt;
  if (elt->client_id) free(elt->client_id);
}

UT_icd childer_node_icd = {sizeof(ChilderNode), NULL, childer_node_copy, childer_node_dtor};

void deduplication(UT_array * array){
    ChilderNode * p = NULL;
    ChilderNode * tmp = NULL;
    utarray_sort(array, structsort);

    for(int i = 0; (p = (ChilderNode*) utarray_next(array, tmp)); i++) {
        if(tmp && !strcmp(p->client_id, tmp->client_id)){
            utarray_erase(array, i, 1);
        }else{
            tmp = p;
        }
    }
}

struct TrieNode * initNode(char * key){
    struct TrieNode *pCrawl;

    HASH_FIND_STR(root.children, key, pCrawl);

    if(pCrawl == NULL){
        pCrawl = (struct TrieNode *) malloc(sizeof(struct TrieNode));
        memset(pCrawl, 0, sizeof * pCrawl);
        pCrawl->key = key;
        pCrawl->childer_node = NULL;
        HASH_ADD_STR(root.children, key, pCrawl);
    }

    return pCrawl;
}

struct TrieNode * initSYSNode(char * key){
    struct TrieNode *pCrawl;

    HASH_FIND_STR(sys.children, key, pCrawl);

    if(pCrawl == NULL){
        pCrawl = (struct TrieNode *) malloc(sizeof(struct TrieNode));
        memset(pCrawl, 0, sizeof * pCrawl);
        pCrawl->key = key;
        pCrawl->childer_node = NULL;
        HASH_ADD_STR(sys.children, key, pCrawl);
    }

    return pCrawl;
}

struct TrieNode * initPlusNode(){
    struct TrieNode *pCrawl;
    HASH_FIND_STR(root.plus_children, "+", pCrawl);

    if(pCrawl == NULL){
        pCrawl = (struct TrieNode *) malloc(sizeof(struct TrieNode));
        memset(pCrawl, 0, sizeof * pCrawl);
        pCrawl->key = "+";
        pCrawl->childer_node = NULL;
        HASH_ADD_STR(root.plus_children, key, pCrawl);
    }

    return pCrawl;
}

struct TrieNode * insert(struct TrieNode * s_root, char * key){
    struct TrieNode *tmpCrawl;
    HASH_FIND_STR(s_root->children, key, tmpCrawl);

    if(tmpCrawl == NULL){
        tmpCrawl = (struct TrieNode *) malloc(sizeof(struct TrieNode));
        memset(tmpCrawl, 0, sizeof * tmpCrawl);
        tmpCrawl->key = key;
        tmpCrawl->childer_node = NULL;
        HASH_ADD_STR(s_root->children, key, tmpCrawl);
    }

    return tmpCrawl;
}

struct TrieNode * insertPlus(struct TrieNode * s_root, char * key){
    struct TrieNode *tmpCrawl;
    HASH_FIND_STR(s_root->plus_children, key, tmpCrawl);

    if(tmpCrawl == NULL){
        tmpCrawl = (struct TrieNode *) malloc(sizeof(struct TrieNode));
        memset(tmpCrawl, 0, sizeof * tmpCrawl);
        tmpCrawl->key = key;
        tmpCrawl->childer_node = NULL;
        HASH_ADD_STR(s_root->plus_children, key, tmpCrawl);
    }

    return tmpCrawl;
}

void intercept(char * key, int max_qos, char * client_id){
    struct TrieNode *pCrawl;
    ChilderNode * ic = (ChilderNode *) malloc(sizeof(ChilderNode));
    char * tmp_str;
    int tmp_int = 0;
    int i = 0;

    ic->max_qos = max_qos;
    ic->client_id = (char *) malloc(sizeof(char) * (strlen(client_id) + 1));
    memset(ic->client_id, 0, sizeof(char) * (strlen(client_id) + 1));
    strcpy(ic->client_id, client_id);

    if(key[0] == '#'){
        if(root.childer_node == NULL)
            utarray_new(root.childer_node, &childer_node_icd);

        utarray_push_back(root.childer_node, ic);
        deduplication(root.childer_node);
        return;
    }else if(key[0] == '+'){
        pCrawl = initPlusNode();
        tmp_int = i = 1;
    }else if(key[0] == '/'){
        pCrawl = initNode("/");
        tmp_int = i = 1;
    }else if(key[0] == '$'){
        for(i = 1; key[i] != '\0' && key[i] != '/'; i++);
        if(i == 1){
            pCrawl = initSYSNode("/");
            tmp_int = i = 2;
        }else{    
            tmp_str = (char *) malloc(sizeof(char) * (i));
            memset(tmp_str, 0, sizeof(char) * (i));
            strncpy(tmp_str, &key[1], i - 1);
            tmp_str[i] = '\0';

            printf("tmp_str:%s\n", tmp_str);
            pCrawl = initSYSNode(tmp_str);
        }
    }else{
        for(i = 0; key[i] != '\0' && key[i] != '/'; i++);
        tmp_str = (char *) malloc(sizeof(char) * (i));
        memset(tmp_str, 0, sizeof(char) * (i));
        strncpy(tmp_str, &key[0], i);
        tmp_str[i] = '\0';
        pCrawl = initNode(tmp_str);
    }

    for(; key[i] != '\0'; i++){
        if(key[i] == '/'){
            if(key[i + 1] == '/' || i == 0 || key[i + 1] == '\0'){
                pCrawl = insert(pCrawl, "/");
            }

            tmp_int = i + 1;
        }else{
            for(i; key[i + 1] != '\0' && key[i + 1] != '/'; i++);
            
            tmp_str = (char *) malloc(sizeof(char) * (i - tmp_int + 2));
            memset(tmp_str, 0, sizeof(char) * (i - tmp_int + 2));
            strncpy(tmp_str, &key[tmp_int], i - tmp_int + 1);

            //printf("tmp_str:%s\n", &key[tmp_int]);

            if(!strcmp(tmp_str, "#")){
                goto pound;
            }else if(!strcmp(tmp_str, "+")){
                pCrawl = insertPlus(pCrawl, "+");
            }else{
                pCrawl = insert(pCrawl, tmp_str);
            }
        }
    }

    goto end;

pound:
    if(pCrawl->PoundNode.childer_node == NULL)
        utarray_new(pCrawl->PoundNode.childer_node, &childer_node_icd);

    utarray_push_back(pCrawl->PoundNode.childer_node, ic);
    deduplication(pCrawl->PoundNode.childer_node);

    return;
end:
    if(pCrawl->childer_node == NULL)
        utarray_new(pCrawl->childer_node, &childer_node_icd);
    
    utarray_push_back(pCrawl->childer_node, ic);
    deduplication(pCrawl->childer_node);
}

UT_array * array_hand(UT_array * array, char * key){
    struct TrieNode * p = NULL;
    struct TrieNode * tmpCrawl = NULL;
    UT_array * pound_array = NULL;
    UT_array * ans_array = NULL;

    utarray_new(pound_array, &childer_node_icd);
    utarray_new(ans_array, &node_icd);

    for(int i = 0; (p = (struct TrieNode *) utarray_next(array, p)); i++){
        HASH_FIND_STR(p->children, key, tmpCrawl);

        if(p->plus_children != NULL)
            utarray_push_back(ans_array, p->plus_children);

        if(p->PoundNode.childer_node != NULL)
            utarray_concat(pound_array, p->PoundNode.childer_node);

        if(tmpCrawl != NULL){
            utarray_push_back(ans_array, tmpCrawl);
        }
    }

    utarray_clear(array);
    utarray_concat(array, ans_array);

    return pound_array;
}

UT_array * search(char * key){
    UT_array * pCrawl = NULL;
    UT_array * tmp_array = NULL;
    UT_array * test_array = NULL;

    struct TrieNode *tmpCrawl = NULL;
    struct TrieNode * p = NULL;

    ChilderNode * test = NULL;
    char * tmp_str = NULL;
    int tmp_int = 0;
    int i = 0;

    utarray_new(tmp_array, &childer_node_icd);
    utarray_new(pCrawl, &node_icd);

    if(root.childer_node != NULL && key[0] != '$')
        if(utarray_front(root.childer_node) != NULL){
            utarray_concat(tmp_array, root.childer_node);
        }

    if(key[0] == '/'){
        HASH_FIND_STR(root.children, "/", tmpCrawl);
        tmp_int = i = 1;
    }else if(key[0] == '$'){
        for(i = 1; key[i] != '\0' && key[i] != '/'; i++);
        if(i == 1){
            HASH_FIND_STR(sys.children, "/", tmpCrawl);
            tmp_int = i = 2;
        }else{    
            tmp_str = (char *) malloc(sizeof(char) * (i));
            memset(tmp_str, 0, sizeof(char) * (i));
            strncpy(tmp_str, &key[1], i - 1);
            tmp_str[i] = '\0';
            HASH_FIND_STR(sys.children, tmp_str, tmpCrawl);
        }
    }else{
        for(i = 0; key[i] != '\0' && key[i] != '/'; i++);
        tmp_str = (char *) malloc(sizeof(char) * (i + 1));
        memset(tmp_str, 0, sizeof(char) * i);
        strncpy(tmp_str, &key[0], i);
        tmp_str[i] = '\0';
        HASH_FIND_STR(root.children, tmp_str, tmpCrawl);
    }

    if(tmpCrawl != NULL)
        utarray_push_back(pCrawl, tmpCrawl);

    if(root.plus_children != NULL)
        utarray_push_back(pCrawl, root.plus_children);

    if(utarray_front(pCrawl) != NULL){
        for(; key[i] != '\0'; i++){
            if(key[i] == '/'){
                if(key[i + 1] == '/' || i == 0 || key[i + 1] == '\0'){
                    if((test_array = array_hand(pCrawl, "/")) != NULL)
                        utarray_concat(tmp_array, test_array);
                }

                tmp_int = i + 1;
            }else{
                for(i; key[i + 1] != '\0' && key[i + 1] != '/'; i++);
                tmp_str = (char *) malloc(sizeof(char) * (i - tmp_int + 1));
                memset(tmp_str, 0, sizeof(char) * (i - tmp_int + 1));
                strncpy(tmp_str, &key[tmp_int], i - tmp_int + 1);

                if((test_array = array_hand(pCrawl, tmp_str)) != NULL)
                    utarray_concat(tmp_array, test_array);
            }
        }
    }

    while((p = (struct TrieNode *) utarray_next(pCrawl, p))){
        if(p->childer_node != NULL){
            utarray_concat(tmp_array, p->childer_node);
        }else if(p->PoundNode.childer_node != NULL){
            utarray_concat(tmp_array, p->PoundNode.childer_node);
        }
    }

    deduplication(tmp_array);

    return tmp_array;
}

void delete_client_id(UT_array * array, char * client_id){
    ChilderNode *first, *find;
    long int pos = 0;

    if((find = utarray_find(array, client_id, struct_str_sort)) != NULL){
        first = utarray_front(array);
        pos = find - first;

        utarray_erase(array, pos, 1);
    }
}

void delete_node(struct TrieNode * node, char * key, char * client_id){
    struct TrieNode * out_node = NULL;
    int key_len = strlen(key);
    int tmp_int = 0;
    char * tmp_str;
    int i = 0;

    if(key_len <= 0){
        delete_client_id(node->childer_node, client_id);
    }

    if(key[0] == '/'){
        tmp_str = (char *) malloc(sizeof(char) * (key_len + 1));
        memset(tmp_str, 0, sizeof(char) * (key_len + 1));
        memmove(tmp_str, &key[1], key_len - 1);

        if(key[1] == '/' || key[1] == '\0'){
            HASH_FIND_STR(node->children, "/", out_node);
            if(out_node == NULL)
                return;
            else{
                delete_node(out_node, tmp_str, client_id);
            }
        }else{
            delete_node(node, tmp_str, client_id);
        }
    }else{
        for(i = 0; key[i + 1] != '\0' && key[i + 1] != '/'; i++);
        tmp_str = (char *) malloc(sizeof(char) * (i - tmp_int + 1));
        memset(tmp_str, 0, sizeof(char) * (i - tmp_int + 1));
        strncpy(tmp_str, &key[tmp_int], i - tmp_int + 1);

        if(!strcmp(tmp_str, "#")){
            delete_client_id(node->PoundNode.childer_node, client_id);
            return;
        }else if(!strcmp(tmp_str, "+")){
            HASH_FIND_STR(node->plus_children, "+", out_node);
            if(out_node == NULL)
                return;
        }else{
            HASH_FIND_STR(node->children, tmp_str, out_node);
            if(out_node == NULL)
                return;
        }

        tmp_str = (char *) malloc(sizeof(char) * (key_len + 1));
        memmove(tmp_str, &key[i + 1], key_len);
        delete_node(out_node, tmp_str, client_id);
    }

    if(out_node != NULL){
        DELETE(node, out_node);
    }
}

void delete_topic(char * key, char * client_id){
    struct TrieNode * node = NULL;

    char * tmp_str = NULL;
    ChilderNode * first, * find;
    int key_len = strlen(key);
    int i = 0;
    long int pos = 0;

    if(key[0] == '#'){
        if((find = utarray_find(root.childer_node, client_id, struct_str_sort)) != NULL){
            first = utarray_front(root.childer_node);
            pos = find - first;

            utarray_erase(root.childer_node, pos, 1);
        }

        return;
    }else if(key[0] == '+'){
        HASH_FIND_STR(root.plus_children, "+", node);

        i = 1;
    }else if(key[0] == '/'){
        HASH_FIND_STR(root.children, "/", node);

        i = 1;
    }else if(key[0] == '$'){
        for(i = 1; key[i] != '\0' && key[i] != '/'; i++);
        if(i == 1){
            HASH_FIND_STR(sys.children, "/", node);
            i = 2;
        }else{    
            tmp_str = (char *) malloc(sizeof(char) * (i));
            memset(tmp_str, 0, sizeof(char) * (i));
            strncpy(tmp_str, &key[1], i - 1);
            tmp_str[i] = '\0';
            HASH_FIND_STR(sys.children, tmp_str, node);
        }
    }else{
        for(i = 0; key[i] != '\0' && key[i] != '/'; i++);
        
        tmp_str = (char *) malloc(sizeof(char) * (i));
        memset(tmp_str, 0, sizeof(char) * (i));
        strncpy(tmp_str, &key[0], i);
        tmp_str[i] = '\0';
        
        HASH_FIND_STR(root.children, tmp_str, node);
    }

    if(node != NULL){
        tmp_str = (char *) malloc(sizeof(char) * (key_len + 1));
        memset(tmp_str, 0, sizeof(char) * (key_len + 1));
        memmove(tmp_str, &key[i], key_len - i);

        delete_node(node, tmp_str, client_id);

        if(key[0] == '$'){
            DELETE(&sys, node);
        }else{
            DELETE(&root, node);
        }
    }
}

void delete_all(struct TrieNode * node){
    struct TrieNode *current_user, *tmp;

    if(node == NULL){
        return;
    }

    HASH_ITER(hh, node, current_user, tmp){
        if(current_user->childer_node != NULL){
            utarray_clear(current_user->childer_node);
        }

        if(current_user->PoundNode.childer_node != NULL){
            utarray_clear(current_user->childer_node);
        }

        if(current_user->plus_children != NULL){
            delete_all(current_user->plus_children);
        }

        delete_all(current_user->children);
    }

    HASH_CLEAR(hh, node);
}

void printf_all(struct TrieNode * s_root){
    struct TrieNode *current_user, *tmp;
    ChilderNode *p1 = NULL;
    ChilderNode *p2 = NULL;

    if(s_root == NULL){
        return;
    }

    HASH_ITER(hh, s_root, current_user, tmp){
        if(current_user->childer_node != NULL){
            printf("%s ", current_user->key);
            while((p1=(ChilderNode*) utarray_next(current_user->childer_node,p1))){
                printf("client_id:%s ", p1->client_id);
                printf("max_qos:%d ", p1->max_qos);
            }
        }else{
            printf("%s ", current_user->key);
        }

        if(current_user->PoundNode.childer_node != NULL){
            printf("\n---------------pound------------\n");
            printf("%s ", current_user->key);
            while((p2=(ChilderNode *)utarray_next(current_user->PoundNode.childer_node, p2))){
                printf("client_id:%s ", p2->client_id);
                printf("max_qos:%d", p2->max_qos);
            }
            printf("\n");
        }

        if(current_user->plus_children != NULL){
            printf_all(current_user->plus_children);
        }

        printf_all(current_user->children);
    }
}
