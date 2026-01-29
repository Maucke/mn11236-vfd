#ifndef LLIST_H
#define LLIST_H

#include <stddef.h>

typedef struct LListNode {
    struct LListNode* next;
    void* data;
} LListNode;

typedef struct {
    LListNode* head;
} LList;

void llist_init(LList* list);
void llist_add(LList* list, void* data);
int  llist_remove(LList* list, void* data); // 根据 data 地址删除节点
void llist_clear(LList* list);

#endif

