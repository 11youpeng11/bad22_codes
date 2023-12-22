#ifndef WATCHPOINT_H
#define WATCHPOINT_H

#include "sdb.h"
#include <common.h>
#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  
  /*2023/12/12 */
	bool flag;
	char expr[256];
	
	int old_value;

} WP;


extern WP wp_pool[NR_WP];
extern WP* head __attribute__((unused));
extern WP* free_ __attribute__((unused));

void init_wp_pool();
WP* new_wp();
void free_wp(WP *wp);


#endif
