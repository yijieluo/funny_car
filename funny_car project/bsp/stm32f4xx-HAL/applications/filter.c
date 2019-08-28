#include "filter.h"
#include <stdlib.h>
#include <string.h>

struct list* header = RT_NULL;
rt_uint8_t list_len = 0;

/*循环链表，滑动窗口滤波*/
rt_int8_t list_creat(filter_data_type value[])
{
	header = (struct list *)malloc(sizeof(struct list));
	memcpy(header->buf, value, sizeof(filter_data_type)*DATA_WIDTH);
	header->next_node = header;
	list_len = 1;
	return 0;
}
rt_int8_t list_push(filter_data_type value[])
{
	struct list* p_list = header;
	if (list_len < MAX_LIST_LEN)
	{
		for (int i = 1; i < list_len; i++)
		{
			p_list = p_list->next_node;
		}
		struct list* tail = (struct list *)malloc(sizeof(struct list));
		memcpy(tail->buf, value, sizeof(filter_data_type)*DATA_WIDTH);
		tail->next_node = header;
		p_list->next_node = tail;
		list_len++;
		return 0;
	}
	memcpy(p_list->buf, value, sizeof(filter_data_type)*DATA_WIDTH);
	header = header->next_node;
	return 0;
}
void list_avr(filter_data_type result[])
{
	filter_data_type sum[DATA_WIDTH] = {0};
	struct list* p_list = header;
	for(int i=0; i<MAX_LIST_LEN; i++)
	{
		for(int j=0; j<DATA_WIDTH; j++)
		{
			sum[j] += p_list->buf[j];
		}
		p_list = p_list->next_node;
	}
	for(int j=0; j<DATA_WIDTH; j++)
	{
		result[j] = sum[j] / MAX_LIST_LEN;
	}
}
