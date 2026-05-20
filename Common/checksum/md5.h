#pragma once

///MD5的结果数据长度
#define MD5_HASH_SIZE (16)
///SHA1的结果数据长度
#define SHA1_HASH_SIZE (20)


//每次处理的BLOCK的大小
#define ZEN_MD5_BLOCK_SIZE  (64)

//md5算法的上下文，保存一些状态，中间数据，结果
typedef struct md5_ctx
{
	//处理的数据的长度
	unsigned __int64 length_;
	//还没有处理的数据长度
	unsigned __int64 unprocessed_;
	//取得的HASH结果（中间数据）
	DWORD  hash_[4];

	unsigned char remains[ZEN_MD5_BLOCK_SIZE];
	int nRemains;
} md5_ctx;




/*!
@brief      求某个内存块的MD5，
@return     unsigned char* 返回的的结果，
@param[in]  buf    求MD5的内存BUFFER指针
@param[in]  size   BUFFER长度
@param[out] result 结果
*/
extern unsigned char *md5(const unsigned char *buf,
				   size_t size,
				   unsigned char result[MD5_HASH_SIZE]);

extern void md5_begin(md5_ctx *ctx);
extern void md5_update(md5_ctx *ctx,const unsigned char *buf,size_t size);
extern void md5_finalize(md5_ctx *ctx,unsigned char result[MD5_HASH_SIZE]);



/*!
@brief      求内存块BUFFER的SHA1值
@return     unsigned char* 返回的的结果
@param[in]  buf    求SHA1的内存BUFFER指针
@param[in]  size   BUFFER长度
@param[out] result 结果
*/
extern unsigned char *sha1(const unsigned char *buf,
					size_t size,
					unsigned char result[SHA1_HASH_SIZE]);


