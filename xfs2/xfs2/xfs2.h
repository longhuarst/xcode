#ifndef __xfs2_h
#define __xfs2_h


#include <stdint.h>
#include <stdbool.h>




typedef struct {
	uint32_t wr_index[5];//дָ��
	uint32_t rd_index[5];//��ָ��
	uint32_t initcode[5];//��ʼ������

	struct {
		uint32_t address[5];//���ַ
		uint32_t length[5];//���С
	}store[65536];//DDR  -- ��ECC

}xfs2_ddr_type;



typedef struct {

	uint16_t wr_index;//Ƭ��
	uint16_t rd_index;//Ƭ��
	uint32_t initcode;//��ʼ������
	xfs2_ddr_type *ddr;//DDR

	uint32_t max_block;//������
	bool over_flow;//���
}xfs2_type;

extern bool xfs2_push(xfs2_type *xfs2, uint32_t length, uint32_t *address);
extern bool xfs2_pop(xfs2_type *xfs2, uint32_t *length, uint32_t *address);






#endif
