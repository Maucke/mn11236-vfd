#include "llist.h"
#include <stdlib.h>

// 初始化链表
void llist_init(LList* list) {
    if (list == NULL) return;
    list->head = NULL;
}

// 添加节点到链表尾部
void llist_add(LList* list, void* data) {
    if (list == NULL) return;
    
    LListNode* newNode = (LListNode*)malloc(sizeof(LListNode));
    if (newNode == NULL) return; // 内存分配失败
    
    newNode->data = data;
    newNode->next = NULL;
    
    // 如果链表为空，新节点就是头节点
    if (list->head == NULL) {
        list->head = newNode;
        return;
    }
    
    // 找到链表尾部
    LListNode* current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }
    
    // 添加新节点到尾部
    current->next = newNode;
}

// 根据data地址删除节点
int llist_remove(LList* list, void* data) {
    if (list == NULL || list->head == NULL) return 0;
    
    LListNode* current = list->head;
    LListNode* previous = NULL;
    
    // 遍历链表查找匹配data地址的节点
    while (current != NULL) {
        if (current->data == data) {
            // 处理头节点的情况
            if (previous == NULL) {
                list->head = current->next;
            } else {
                previous->next = current->next;
            }
            
            free(current);
            return 1; // 删除成功
        }
        
        previous = current;
        current = current->next;
    }
    
    return 0; // 未找到匹配节点
}

// 清空链表
void llist_clear(LList* list) {
    if (list == NULL) return;
    
    LListNode* current = list->head;
    LListNode* next;
    
    // 遍历链表，释放每个节点
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    
    list->head = NULL;
}    
