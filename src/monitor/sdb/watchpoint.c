/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#define NR_WP 32
#include "watchpoint.h"

/*
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  // TODO: Add more members if necessary
  
  
	bool flag;
	char expr[256];
	int new_value;
	int old_value;

} WP;

static WP wp_pool[NR_WP] = {};		//思考题 为什么呢？静态 非静态声明 如果不该编译一直报错
static WP *head = NULL, *free_ = NULL;
*/
WP wp_pool[NR_WP] = {};
WP *head = NULL, *free_ = NULL;


void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = ((i == NR_WP - 1 ) ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
/*
WP* new_wp(){
	for(WP* p = free_ ; p -> next != NULL ; p = p -> next){
		if(p -> flag == false){
			p -> flag = true;
			if(head == NULL){
				head = p;
			}
			return p;
		}
	}
	printf("No unuse point. \n");
	assert(0);
	return NULL;
}
*/
WP* new_wp() {
    if (free_ == NULL) {
        printf("No unused watchpoint.\n");
        assert(0);
    }

    WP* wp = free_;
    free_ = free_->next;

    wp->flag = true;
    wp->next = NULL;

    if (head == NULL) {
        head = wp;
    } else {
        WP* p = head;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = wp;
    }

    return wp;
}

void free_wp(WP *wp){
	if(head -> NO == wp -> NO){
		head -> flag = false;
	//	head = NULL;
		printf("Delete watchpoint success.\n" );
		return ;
	}
	for(WP* p = head; p -> next != NULL; p = p -> next){
		if(p -> next -> NO == wp -> NO){
		//	p -> next = p -> next -> next;
		//	p -> next -> flag = false;
			
			WP* nextNode = p->next;
			p->next = nextNode->next;
			nextNode->flag = false;
			printf("free success.\n");
			return ;
		}
	}
}
/*
void free_wp(WP *wp) {
    // 检查是否有要释放的监视点
    if (head == NULL) {
        printf("没有要释放的监视点。\n");
        assert(0);
    }

    // 检查头节点是否是要释放的监视点
    if (head->NO == wp->NO) {
        head = wp->next;
    } else {
        WP* p = head;
        // 查找要释放的监视点的前一个节点
        while (p->next != NULL && p->next->NO != wp->NO) {
            p = p->next;
        }
        // 检查要释放的监视点是否无效
        if (p->next == NULL) {
            printf("要释放的监视点无效。\n");
            assert(0);
        }
        // 从链表中移除要释放的监视点
        p->next = wp->next;
    }

    // 将监视点标记为未使用
    wp->flag = false;
    // 将监视点添加到空闲链表的头部
    wp->next = free_;
    free_ = wp;
}
*/

