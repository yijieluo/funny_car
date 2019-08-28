#ifndef _FILTER_H
#define _FILTER_H
#include <rtdef.h>

typedef int16_t filter_data_type;

struct list
{
	filter_data_type buf[2];
	struct list* next_node;
};
#define MAX_LIST_LEN 3//窗口宽度5
#define DATA_WIDTH 2//左右轮转速
rt_int8_t list_creat(filter_data_type value[]);
rt_int8_t list_push(filter_data_type value[]);
void list_avr(filter_data_type *);

#endif
