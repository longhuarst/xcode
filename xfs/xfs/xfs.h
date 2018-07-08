#ifndef __xfs_h
#define __xfs_h

#include <stdint.h>
#include <stdbool.h>







typedef struct
{
	uint32_t address[4];
	uint32_t length[4];
}xfs_store_ecc_type;

typedef struct
{
	uint32_t wr[4];
	uint32_t rd[4];
}xfs_index_ecc_type;


typedef struct
{
	uint32_t code[4];
}xfs_init_ecc_type;

typedef struct
{	
	xfs_init_ecc_type* init_cache;//512Bytes 1Groups
	xfs_index_ecc_type* index_cache;//512Bytes 1Groups
	xfs_store_ecc_type* wr_cache;//512Bytes cache  16groups
	xfs_store_ecc_type* rd_cache;//512Bytes cache  16groups
}xfs_type;


int xfs_init(xfs_type *xfs, uint32_t initcode);





#endif // __xfs_h
