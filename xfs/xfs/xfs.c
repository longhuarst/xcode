#include "xfs.h"



uint32_t __xfs_get_initcode() 
{
	uint32_t initcode;

	return initcode;
}

void __xfs_set_initcode(uint32_t initcode)
{

}


void __xfs_sync_cache_to_disk(xfs_type *xfs)
{
	//��ddr��ַ��ͬ�����洢��
}


void __xfs_sync_cache_from_disk(xfs_type *xfs)
{
	//�Ӵ洢��ͬ����ddr

}





void __xfs_sync_wr_cache_from_disk(xfs_type *xfs)
{
	//�Ӵ洢��ͬ����ddr

}


void __xfs_sync_wr_cache_to_disk(xfs_type *xfs)
{
	//�Ӵ洢��ͬ����ddr

}















void xfs_ecc_set_real(uint32_t *address, uint32_t data)
{
	*address = data;
	*(address + 4) = data;
	*(address + 8) = data;
	*(address + 12) = data;

}

uint32_t xfs_ecc_get_real(uint32_t *pData)
{
	uint8_t cnt[4] = { 0,0,0,0 };//��¼����

	//�����
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (pData[j] == pData[i])
				cnt[i]++;
		}
	}
	
	uint8_t index = 0;
	//��ѯ������
	for (int i = 1; i < 4; ++i) {
		if (cnt[index] < cnt[i]) {
			index = i;
		}
	}

	return pData[index];
}



void xfs_set_initcode(xfs_type *xfs, uint32_t initcode)
{
	xfs_ecc_set_real(&xfs->init_cache->code[0], initcode);
}



uint32_t xfs_get_initcode(xfs_type *xfs)
{
	return xfs_ecc_get_real(&xfs->init_cache->code[0]);
}



void xfs_set_wr_index(xfs_type *xfs, uint32_t wr_index)
{
	xfs_ecc_set_real(&xfs->index_cache->wr[0], wr_index);
}



uint32_t xfs_get_wr_index(xfs_type *xfs)
{
	return xfs_ecc_get_real(&xfs->index_cache->wr[0]);
}




void xfs_set_rd_index(xfs_type *xfs, uint32_t rd_index)
{
	xfs_ecc_set_real(&xfs->index_cache->rd[0], rd_index);
}



uint32_t xfs_get_rd_index(xfs_type *xfs)
{
	return xfs_ecc_get_real(&xfs->index_cache->rd[0]);
}








int xfs_init(xfs_type *xfs, uint32_t initcode, uint32_t ddr_address)
{
	//�Ӵ洢��ͬ�����ݳ�����DDR
	__xfs_sync_cache_from_disk(xfs);

	//ƥ���ʼ�������Ƿ��Ǻ�
	if (__xfs_get_initcode != initcode) {
		//���Ǻϣ���Ҫ���³�ʼ���ļ��ṹ
		xfs->init_cache = ddr_address + 512 * 0;
		xfs->index_cache = ddr_address + 512 * 1;
		xfs->wr_cache = ddr_address + 512 * 2;
		xfs->rd_cache = ddr_address + 512 * 3;


		xfs_set_wr_index(xfs, 0);
		xfs_set_rd_index(xfs, 0);
		xfs_set_initcode(xfs, initcode);

		__xfs_sync_cache_to_disk(xfs);//������ͬ�����洢��
	}

	return 0;
}



bool xfs_is_full(xfs_type *xfs)
{
	uint32_t wr_index = xfs_get_wr_index(xfs);
	uint32_t rd_index = xfs_get_rd_index(xfs);

	return (wr_index - rd_index + 65536) % 65536 == 65535;
}


bool xfs_is_empty(xfs_type *xfs)
{
	uint32_t wr_index = xfs_get_wr_index(xfs);
	uint32_t rd_index = xfs_get_rd_index(xfs);

	return wr_index == rd_index;
}



void xfs_rd_index_add(xfs_type *xfs)
{
	uint32_t rd_index = xfs_get_rd_index(xfs);
	rd_index++;
	rd_index %= 65536;
	xfs_set_rd_index(xfs, rd_index);
}


void xfs_wd_index_add(xfs_type *xfs)
{
	uint32_t wr_index = xfs_get_wr_index(xfs);
	wr_index++;
	wr_index %= 65536;
	xfs_set_wr_index(xfs, wr_index);
}

/*
	����������
*/
void xfs_push(xfs_type *xfs, uint32_t address, uint32_t length)
{
	
	if (xfs_is_full(xfs)) {
		//����
		xfs_rd_index_add(xfs);
	}

	uint32_t wr_index = xfs_get_wr_index(xfs);//��ȡдָ��

	



	xfs_wr_index_add(xfs);


	if (wr_index % 16 == 0) {
		//ÿ16����Ϊһ��cache��
		//��Ҫˢ�»���
		__xfs_sync_wr_cache_to_disk(xfs);//��Ҫ����ǰ��дָ��ͬ����ȥ
	}
}

void xfs_pop()
{

}
