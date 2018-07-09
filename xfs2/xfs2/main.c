
#include <stdlib.h>

#include <stdio.h>

#include "xfs2.h"

#include <time.h>



#define _25GB (25*1024*1024*2)

uint8_t buffer[10 * 1024 * 1024];//10MB ģ��ddr


extern bool xfs2_init(xfs2_type *xfs2, uint32_t *ddr, uint32_t initcode, uint32_t max_block);

xfs2_type xfs2;

int main()
{

	printf("========== xfs v2.0 ����ϵͳ ==========\r\n");


	//randomize();

	

	while (!xfs2_init(&xfs2, (uint32_t *)buffer, 0xaa557788, _25GB)) {
		//��ʼ��ʧ��

		printf("��ʼ��ʧ�ܣ�\r\n");
	}


	printf("��ʼ���ɹ�\r\n");



	//��ʼģ��Ŷд��

	for (int i = 0; i < 80000; ++i) {
		uint32_t address;
		uint32_t length = 800 * 2 + rand() % 100; //

		printf("��ʼ�������� ���� = %d.%d KB\r\n", length / 2, length % 2 * 5);
		if (xfs2_push(&xfs2, length, &address)) {
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!����������� ��ַ = %d\r\n", address);
		}
		else {
			printf("����ʧ�ܣ�\r\n");
		}
		
	}


	int cnt = 0;
	int last_address = 0;
	int length;


	int cnt_err = 0;

	while (1) {

		if (!xfs2_pop(&xfs2, &length, &last_address)) {
			break;
		}


		last_address += length;
		while (1) {
			uint32_t length;
			uint32_t address;

			if (xfs2_pop(&xfs2, &length, &address)) {
				//printf("��ַ = %d ���� = %d,  �´�д��ַ = %d", address, length);
				cnt++;
				if (last_address == address) {
					last_address += length;

				}
				else {
					printf("��⵽��ַ��ƥ��  %d    Ӧ��Ϊ =%d  ʵ��Ϊ %d", cnt, last_address, address);
					cnt_err++;
					break;
				}

				printf("\r\n");


				if (cnt % 1000 == 0) {
					//system("pause");
				}

			}
			else {
				break;
			}

		}

	}

	

	printf("ƥ�����   ƥ����Ŀ = %d  ��ƥ���� = %d\r\n", cnt, cnt_err);














	system("pause");


	return 0;
}