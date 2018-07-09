#ifndef __xfs2_h
#define __xfs2_h


#include <stdint.h>
#include <stdbool.h>




typedef struct {
	uint32_t wr_index[5];//写指针
	uint32_t rd_index[5];//读指针
	uint32_t initcode[5];//初始化代码

	struct {
		uint32_t address[5];//块地址
		uint32_t length[5];//块大小
	}store[65536];//DDR  -- 带ECC

}xfs2_ddr_type;



typedef struct {

	uint16_t wr_index;//片内
	uint16_t rd_index;//片内
	uint32_t initcode;//初始化代码
	xfs2_ddr_type *ddr;//DDR

	uint32_t max_block;//最大块数
	bool over_flow;//溢出
}xfs2_type;

extern bool xfs2_push(xfs2_type *xfs2, uint32_t length, uint32_t *address);
extern bool xfs2_pop(xfs2_type *xfs2, uint32_t *length, uint32_t *address);






#endif
