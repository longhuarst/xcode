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
	uint8_t cnt[5] = { 0,0,0,0 ,0 };//记录概率
									//求概率
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			if (pData[j] == pData[i])
				cnt[i]++;
		}
	}
	uint8_t index = 0;
	//查询最大概率
	for (int i = 1; i < 5; ++i) {
		if (cnt[index] < cnt[i]) {
			index = i;
		}
	}
	return pData[index];
}







//同步到存储器
void xfs2_sysnc_to_disk(xfs2_type *xfs2)
{

}


//同步到DDR
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



//设置 存储的结构数据    xfs2对象   游标    地址  长度
void xfs2_set_data(xfs2_type *xfs2, uint16_t index, uint32_t address, uint32_t length)
{
	xfs2_ecc_set_real(&xfs2->ddr->store[index].address[0], address);
	xfs2_ecc_set_real(&xfs2->ddr->store[index].length[0], length);
}


//获取 存储的结构数据    xfs2对象   游标    地址  长度
void xfs2_get_data(xfs2_type *xfs2, uint16_t index, uint32_t *address, uint32_t *length)
{
	*address = xfs2_ecc_get_real(&xfs2->ddr->store[index].address[0]);
	*length = xfs2_ecc_get_real(&xfs2->ddr->store[index].length[0]);
}


//初始化
bool xfs2_init(xfs2_type *xfs2,uint32_t *ddr,uint32_t initcode, uint32_t max_block)
{
	xfs2->ddr = (xfs2_ddr_type *)ddr;

	xfs2_sync_from_disk(xfs2);//从硬盘同步

	xfs2_all_sync_from_ddr(xfs2);//从DDR同步到内部ram

	if (initcode != xfs2->initcode) {

		debug("初始化代码不匹配\r\n");

		//初始化代码不匹配
		xfs2->initcode = initcode;
		xfs2->wr_index = 0;
		xfs2->rd_index = 0;
		xfs2_all_sync_to_ddr(xfs2);//同步到DDR
		//暂时不同步到存储器
		return false;
	}

	//如果匹配则什么都不做

	xfs2->max_block = max_block;//最大块数

	xfs2->over_flow = false;//设置未溢出

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


//追加
//提供长度（byte） 返回EMMC地址（32位的）
bool xfs2_push(xfs2_type *xfs2, uint32_t length, uint32_t *address)
{
	if (address == NULL || xfs2 == NULL)
		return false;
	//首先要判断是否为空
	if (xfs2_is_empty(xfs2)) {
		//空的
		//直接插入 
		*address = 0;//存储器地址从0开始
		xfs2_set_data(xfs2,xfs2->wr_index, 0, length);//设置数据
		xfs2->wr_index++;//写游标增加

		debug("##插入成功 -- xfs 为空\r\n");
		debug("插入地址 = 0 , 插入的长度 = %d\r\n", length);

		return true;//返回成功
	}
	//判断当前缓存的个数
	if (xfs2_length(xfs2) == 1) {
		//只有一个 //判断增加数据后是否会溢出
		//第一组数据肯定是从0位置开始的 所以只需要计算是否会上溢出就可以

		//取出 读指针 处的地址 和 长度
		uint32_t rd_address, rd_length;
		xfs2_get_data(xfs2, xfs2->rd_index, &rd_address, &rd_length);
		
		//判断写入新文件后是否会溢出
		//0+块长度R + 写入的块长度 
		if (rd_length + length > xfs2->max_block) {
			//溢出了 需要删掉第一个数据 
			xfs2_set_data(xfs2, xfs2->rd_index, 0, length);
			*address = 0;
			xfs2->over_flow = true;//溢出标志

			debug("#######11111111#######插入地址 = 0 , 插入的长度 = %d\r\n", length);

			return true;
		}
		else {
			xfs2_set_data(xfs2, xfs2->wr_index, rd_address+rd_length, length);
			xfs2->wr_index++;
			*address = rd_address + rd_length;



			debug("#######22222222$###########插入地址 = %d , 插入的长度 = %d\r\n", *address,length);
			return true;
		}
		
	}
	if (!xfs2_is_full(xfs2)) {
		//没有满

		printf("###########当前拥有的元素个数  = %d \r\n", xfs2_length(xfs2));



		//取出 读指针 处的地址 和 长度
		uint32_t rd_address, rd_length;
		xfs2_get_data(xfs2, xfs2->rd_index, &rd_address, &rd_length);

		//取出 上次写指针 处的地址 和 长度
		uint32_t wr_address, wr_length;
		xfs2_get_data(xfs2, xfs2->wr_index-1, &wr_address, &wr_length);


		debug("当前 读地址 = %d 读长度 = %d 写地址 = %d 写长度 = %d\r\n", rd_address, rd_length, wr_address, wr_length);
		debug("当前读游标 = %d , 写游标 = %d \r\n", xfs2->rd_index, xfs2->wr_index);
		//计算存储结构当前状态
		if (wr_address > rd_address) {
			//_________r******************w______

			//这种情况先判断上溢出
			if (wr_address + wr_length + length <= xfs2->max_block) {
				//不溢出
				xfs2_set_data(xfs2, xfs2->wr_index, wr_address + wr_length, length);//设置数据 
				xfs2->wr_index++;//写游标增加
				*address = wr_address + wr_length;
				return true;
			}
			else {
				//上溢出
				//从0开始
				if (length <= rd_address) {
					//没有下溢出
					xfs2_set_data(xfs2, xfs2->wr_index,0, length);//设置数据 
					xfs2->wr_index++;//写游标增加
					*address = 0;
					return true;
				}
				else {
					//下溢出
					//需要删除数据

					//先删除一个最旧的数据
					xfs2->rd_index++;
					xfs2->over_flow = true;//溢出了

					debug("##插入失败  1\r\n");

					return false;//返回失败
				}
			}
		}
		else {
			//************w_____________r*********
			if (wr_address + wr_length + length <= rd_address) {
				//不溢出
				xfs2_set_data(xfs2, xfs2->wr_index, wr_address + wr_length, length);//设置数据 
				xfs2->wr_index++;//写游标增加
				*address = 0;
				return true;
			}
			else {
				//溢出了


				//先删除一个最旧的数据
				xfs2->rd_index++;
				xfs2->over_flow = true;//溢出了
				debug("##插入失败  2\r\n");
				return false;//返回失败
			}
		}
	}
	else {
		//如果游标溢出
		//先删除一个最旧的数据
		xfs2->rd_index++;
		xfs2->over_flow = true;//溢出了
		debug("##插入失败  3\r\n");
		return false;//返回失败
	}
}



bool xfs2_pop(xfs2_type *xfs2, uint32_t *length, uint32_t *address)
{
	if (length == NULL || address == NULL)
		return false;
	//首先判断缓存容量是否充裕
	if (xfs2_is_empty(xfs2)) {
		return false;
	}
	
	//有数据
	//取出 读指针 处的地址 和 长度
	xfs2_get_data(xfs2, xfs2->rd_index, address, length);

	xfs2->rd_index++;
	xfs2->over_flow = false;
	
	return true;
}







































