#include "xfs2.h"

#include <stdio.h>

#define debug printf






void xfs2_ecc_set_real(uint32_t *address, uint32_t data)
{
	address[0] = data;
	address[1] = data;
	address[2] = data;
	address[3] = data;
	address[4] = data;
}

uint32_t xfs2_ecc_get_real(uint32_t *pData)
{
	uint8_t cnt[5] = { 0,0,0,0 ,0 };//��¼����
									//�����
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			if (pData[j] == pData[i])
				cnt[i]++;
		}
	}
	uint8_t index = 0;
	//��ѯ������
	for (int i = 1; i < 5; ++i) {
		if (cnt[index] < cnt[i]) {
			index = i;
		}
	}
	return pData[index];
}







//ͬ�����洢��
void xfs2_sysnc_to_disk(xfs2_type *xfs2)
{

}


//ͬ����DDR
void xfs2_sync_from_disk(xfs2_type *xfs2)
{

}





void xfs2_wr_index_sync_to_ddr(xfs2_type *xfs2)
{
	xfs2_ecc_set_real(&xfs2->ddr->wr_index[0], xfs2->wr_index);
}

void xfs2_wr_index_sync_from_ddr(xfs2_type *xfs2)
{
	xfs2->wr_index = xfs2_ecc_get_real(&xfs2->ddr->wr_index[0]);
}





void xfs2_rd_index_sync_to_ddr(xfs2_type *xfs2)
{
	xfs2_ecc_set_real(&xfs2->ddr->rd_index[0], xfs2->rd_index);
}

void xfs2_rd_index_sync_from_ddr(xfs2_type *xfs2)
{
	xfs2->rd_index = xfs2_ecc_get_real(&xfs2->ddr->rd_index[0]);
}





void xfs2_initcode_sync_to_ddr(xfs2_type *xfs2)
{
	xfs2_ecc_set_real(&xfs2->ddr->initcode[0], xfs2->initcode);
}

void xfs2_initcode_sync_from_ddr(xfs2_type *xfs2)
{
	xfs2->initcode = xfs2_ecc_get_real(&xfs2->ddr->initcode[0]);
}



void xfs2_all_sync_to_ddr(xfs2_type *xfs2)
{
	xfs2_wr_index_sync_to_ddr(xfs2);
	xfs2_rd_index_sync_to_ddr(xfs2);
	xfs2_initcode_sync_to_ddr(xfs2);
}

void xfs2_all_sync_from_ddr(xfs2_type *xfs2)
{
	xfs2_wr_index_sync_from_ddr(xfs2);
	xfs2_rd_index_sync_from_ddr(xfs2);
	xfs2_initcode_sync_from_ddr(xfs2);
}



//���� �洢�Ľṹ����    xfs2����   �α�    ��ַ  ����
void xfs2_set_data(xfs2_type *xfs2, uint16_t index, uint32_t address, uint32_t length)
{
	xfs2_ecc_set_real(&xfs2->ddr->store[index].address[0], address);
	xfs2_ecc_set_real(&xfs2->ddr->store[index].length[0], length);
}


//��ȡ �洢�Ľṹ����    xfs2����   �α�    ��ַ  ����
void xfs2_get_data(xfs2_type *xfs2, uint16_t index, uint32_t *address, uint32_t *length)
{
	*address = xfs2_ecc_get_real(&xfs2->ddr->store[index].address[0]);
	*length = xfs2_ecc_get_real(&xfs2->ddr->store[index].length[0]);
}


//��ʼ��
bool xfs2_init(xfs2_type *xfs2,uint32_t *ddr,uint32_t initcode, uint32_t max_block)
{
	xfs2->ddr = (xfs2_ddr_type *)ddr;

	xfs2_sync_from_disk(xfs2);//��Ӳ��ͬ��

	xfs2_all_sync_from_ddr(xfs2);//��DDRͬ�����ڲ�ram

	if (initcode != xfs2->initcode) {

		debug("��ʼ�����벻ƥ��\r\n");

		//��ʼ�����벻ƥ��
		xfs2->initcode = initcode;
		xfs2->wr_index = 0;
		xfs2->rd_index = 0;
		xfs2_all_sync_to_ddr(xfs2);//ͬ����DDR
		//��ʱ��ͬ�����洢��
		return false;
	}

	//���ƥ����ʲô������

	xfs2->max_block = max_block;//������

	xfs2->over_flow = false;//����δ���

	return true;
}




bool xfs2_is_empty(xfs2_type *xfs2)
{
	return xfs2->wr_index == xfs2->rd_index;
}

bool xfs2_is_full(xfs2_type *xfs2)
{
	return (uint32_t)xfs2->wr_index - xfs2->rd_index + 65536 % 65536 == 65535;
}

uint16_t xfs2_length(xfs2_type *xfs2)
{
	return (uint32_t)xfs2->wr_index - xfs2->rd_index + 65536 % 65536;
}


//׷��
//�ṩ���ȣ�byte�� ����EMMC��ַ��32λ�ģ�
bool xfs2_push(xfs2_type *xfs2, uint32_t length, uint32_t *address)
{
	if (address == NULL || xfs2 == NULL)
		return false;
	//����Ҫ�ж��Ƿ�Ϊ��
	if (xfs2_is_empty(xfs2)) {
		//�յ�
		//ֱ�Ӳ��� 
		*address = 0;//�洢����ַ��0��ʼ
		xfs2_set_data(xfs2,xfs2->wr_index, 0, length);//��������
		xfs2->wr_index++;//д�α�����

		debug("##����ɹ� -- xfs Ϊ��\r\n");
		debug("�����ַ = 0 , ����ĳ��� = %d\r\n", length);

		return true;//���سɹ�
	}
	//�жϵ�ǰ����ĸ���
	if (xfs2_length(xfs2) == 1) {
		//ֻ��һ�� //�ж��������ݺ��Ƿ�����
		//��һ�����ݿ϶��Ǵ�0λ�ÿ�ʼ�� ����ֻ��Ҫ�����Ƿ��������Ϳ���

		//ȡ�� ��ָ�� ���ĵ�ַ �� ����
		uint32_t rd_address, rd_length;
		xfs2_get_data(xfs2, xfs2->rd_index, &rd_address, &rd_length);
		
		//�ж�д�����ļ����Ƿ�����
		//0+�鳤��R + д��Ŀ鳤�� 
		if (rd_length + length > xfs2->max_block) {
			//����� ��Ҫɾ����һ������ 
			xfs2_set_data(xfs2, xfs2->rd_index, 0, length);
			*address = 0;
			xfs2->over_flow = true;//�����־

			debug("#######11111111#######�����ַ = 0 , ����ĳ��� = %d\r\n", length);

			return true;
		}
		else {
			xfs2_set_data(xfs2, xfs2->wr_index, rd_address+rd_length, length);
			xfs2->wr_index++;
			*address = rd_address + rd_length;



			debug("#######22222222$###########�����ַ = %d , ����ĳ��� = %d\r\n", *address,length);
			return true;
		}
		
	}
	if (!xfs2_is_full(xfs2)) {
		//û����

		printf("###########��ǰӵ�е�Ԫ�ظ���  = %d \r\n", xfs2_length(xfs2));



		//ȡ�� ��ָ�� ���ĵ�ַ �� ����
		uint32_t rd_address, rd_length;
		xfs2_get_data(xfs2, xfs2->rd_index, &rd_address, &rd_length);

		//ȡ�� �ϴ�дָ�� ���ĵ�ַ �� ����
		uint32_t wr_address, wr_length;
		xfs2_get_data(xfs2, xfs2->wr_index-1, &wr_address, &wr_length);


		debug("��ǰ ����ַ = %d ������ = %d д��ַ = %d д���� = %d\r\n", rd_address, rd_length, wr_address, wr_length);
		debug("��ǰ���α� = %d , д�α� = %d \r\n", xfs2->rd_index, xfs2->wr_index);
		//����洢�ṹ��ǰ״̬
		if (wr_address > rd_address) {
			//_________r******************w______

			//����������ж������
			if (wr_address + wr_length + length <= xfs2->max_block) {
				//�����
				xfs2_set_data(xfs2, xfs2->wr_index, wr_address + wr_length, length);//�������� 
				xfs2->wr_index++;//д�α�����
				*address = wr_address + wr_length;
				return true;
			}
			else {
				//�����
				//��0��ʼ
				if (length <= rd_address) {
					//û�������
					xfs2_set_data(xfs2, xfs2->wr_index,0, length);//�������� 
					xfs2->wr_index++;//д�α�����
					*address = 0;
					return true;
				}
				else {
					//�����
					//��Ҫɾ������

					//��ɾ��һ����ɵ�����
					xfs2->rd_index++;
					xfs2->over_flow = true;//�����

					debug("##����ʧ��  1\r\n");

					return false;//����ʧ��
				}
			}
		}
		else {
			//************w_____________r*********
			if (wr_address + wr_length + length <= rd_address) {
				//�����
				xfs2_set_data(xfs2, xfs2->wr_index, wr_address + wr_length, length);//�������� 
				xfs2->wr_index++;//д�α�����
				*address = 0;
				return true;
			}
			else {
				//�����


				//��ɾ��һ����ɵ�����
				xfs2->rd_index++;
				xfs2->over_flow = true;//�����
				debug("##����ʧ��  2\r\n");
				return false;//����ʧ��
			}
		}
	}
	else {
		//����α����
		//��ɾ��һ����ɵ�����
		xfs2->rd_index++;
		xfs2->over_flow = true;//�����
		debug("##����ʧ��  3\r\n");
		return false;//����ʧ��
	}
}



bool xfs2_pop(xfs2_type *xfs2, uint32_t *length, uint32_t *address)
{
	if (length == NULL || address == NULL)
		return false;
	//�����жϻ��������Ƿ��ԣ
	if (xfs2_is_empty(xfs2)) {
		return false;
	}
	
	//������
	//ȡ�� ��ָ�� ���ĵ�ַ �� ����
	xfs2_get_data(xfs2, xfs2->rd_index, address, length);

	xfs2->rd_index++;
	xfs2->over_flow = false;
	
	return true;
}







































