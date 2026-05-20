#include "stdh.h"

#include "md5.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

//字节序的小头和大头的问题
#define ZEN_LITTLE_ENDIAN  0x0123
#define ZEN_BIG_ENDIAN     0x3210

//目前所有的代码都是为了小头党服务的，不知道有生之年这套代码是否还会为大头党服务一次？
#ifndef ZEN_BYTES_ORDER
#define ZEN_BYTES_ORDER    ZEN_LITTLE_ENDIAN
#endif

#ifndef ZEN_SWAP_UINT16
#define ZEN_SWAP_UINT16(x)  ((((x) & 0xff00) >>  8) | (((x) & 0x00ff) <<  8))
#endif
#ifndef ZEN_SWAP_UINT32
#define ZEN_SWAP_UINT32(x)  ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
    (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))
#endif
#ifndef ZEN_SWAP_UINT64
#define ZEN_SWAP_UINT64(x)  ((((x) & 0xff00000000000000) >> 56) | (((x) & 0x00ff000000000000) >>  40) | \
    (((x) & 0x0000ff0000000000) >> 24) | (((x) & 0x000000ff00000000) >>  8) | \
    (((x) & 0x00000000ff000000) << 8 ) | (((x) & 0x0000000000ff0000) <<  24) | \
    (((x) & 0x000000000000ff00) << 40 ) | (((x) & 0x00000000000000ff) <<  56))
#endif

//将一个（字符串）数组，拷贝到另外一个DWORD数组，同时每个DWORD反字节序
void *swap_uint32_memcpy(void *to, const void *from, size_t length)
{
    memcpy(to, from, length);
    size_t remain_len =  (4 - (length & 3)) & 3;

    //数据不是4字节的倍数,补充0
    if (remain_len)
    {
        for (size_t i = 0; i < remain_len; ++i)
        {
            *((char *)(to) + length + i) = 0;
        }
        //调整成4的倍数
        length += remain_len;
    }

    //所有的数据反转
    for (size_t i = 0; i < length / 4; ++i)
    {
        ((DWORD *)to)[i] = ZEN_SWAP_UINT32(((DWORD *)to)[i]);
    }

    return to;
}





/*!
@brief      求某个内存块的MD5，
@return     unsigned char* 返回的的结果，
@param[in]  buf    求MD5的内存BUFFER指针
@param[in]  size   BUFFER长度
@param[out] result 结果
*/
unsigned char *md5(const unsigned char *buf,
                   size_t size,
                   unsigned char result[MD5_HASH_SIZE]);


/*!
@brief      求内存块BUFFER的SHA1值
@return     unsigned char* 返回的的结果
@param[in]  buf    求SHA1的内存BUFFER指针
@param[in]  size   BUFFER长度
@param[out] result 结果
*/
unsigned char *sha1(const unsigned char *buf,
                    size_t size,
                    unsigned char result[SHA1_HASH_SIZE]);

//================================================================================================
//MD5的算法

 
 
 #define ROTL32(dword, n) ((dword) << (n) ^ ((dword) >> (32 - (n))))
 #define ROTR32(dword, n) ((dword) >> (n) ^ ((dword) << (32 - (n))))
 #define ROTL64(qword, n) ((qword) << (n) ^ ((qword) >> (64 - (n))))
 #define ROTR64(qword, n) ((qword) >> (n) ^ ((qword) << (64 - (n))))
 
 
 /*!
 @brief      内部函数，初始化MD5的context，内容
 @param      ctx
 */
 static void zen_md5_init(md5_ctx *ctx)
 {
     ctx->length_ = 0;
     ctx->unprocessed_ = 0;
	 ctx->nRemains=0;
 
     /* initialize state */
     ctx->hash_[0] = 0x67452301;
     ctx->hash_[1] = 0xefcdab89;
     ctx->hash_[2] = 0x98badcfe;
     ctx->hash_[3] = 0x10325476;
 }
 
 /* First, define four auxiliary functions that each take as input
  * three 32-bit words and returns a 32-bit word.*/
 
 /* F(x,y,z) = ((y XOR z) AND x) XOR z - is faster then original version */
 #define MD5_F(x, y, z) ((((y) ^ (z)) & (x)) ^ (z))
 #define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
 #define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
 #define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))
 
 /* transformations for rounds 1, 2, 3, and 4. */
 #define MD5_ROUND1(a, b, c, d, x, s, ac) { \
         (a) += MD5_F((b), (c), (d)) + (x) + (ac); \
         (a) = ROTL32((a), (s)); \
         (a) += (b); \
     }
 #define MD5_ROUND2(a, b, c, d, x, s, ac) { \
         (a) += MD5_G((b), (c), (d)) + (x) + (ac); \
         (a) = ROTL32((a), (s)); \
         (a) += (b); \
     }
 #define MD5_ROUND3(a, b, c, d, x, s, ac) { \
         (a) += MD5_H((b), (c), (d)) + (x) + (ac); \
         (a) = ROTL32((a), (s)); \
         (a) += (b); \
     }
 #define MD5_ROUND4(a, b, c, d, x, s, ac) { \
         (a) += MD5_I((b), (c), (d)) + (x) + (ac); \
         (a) = ROTL32((a), (s)); \
         (a) += (b); \
     }
 
 
 /*!
 @brief      内部函数，将64个字节，16个DWORD的数组进行摘要（杂凑）处理，处理的数据自己序是小头数据
 @param      state 存放处理的hash数据结果
 @param      block 要处理的block，64个字节，16个DWORD的数组
 */
 static void zen_md5_process_block(DWORD state[4], const DWORD block[ZEN_MD5_BLOCK_SIZE / 4])
 {
     register unsigned a, b, c, d;
     a = state[0];
     b = state[1];
     c = state[2];
     d = state[3];
 
     const DWORD *x = NULL;
 
     //MD5里面计算的数据都是小头数据.大头党的数据要处理
 #if ZEN_BYTES_ORDER == ZEN_LITTLE_ENDIAN
     x = block;
 #else
     DWORD swap_block[ZEN_MD5_BLOCK_SIZE / 4];
     swap_uint32_memcpy(swap_block, block, 64);
     x = swap_block;
 #endif
 
 
     MD5_ROUND1(a, b, c, d, x[ 0],  7, 0xd76aa478);
     MD5_ROUND1(d, a, b, c, x[ 1], 12, 0xe8c7b756);
     MD5_ROUND1(c, d, a, b, x[ 2], 17, 0x242070db);
     MD5_ROUND1(b, c, d, a, x[ 3], 22, 0xc1bdceee);
     MD5_ROUND1(a, b, c, d, x[ 4],  7, 0xf57c0faf);
     MD5_ROUND1(d, a, b, c, x[ 5], 12, 0x4787c62a);
     MD5_ROUND1(c, d, a, b, x[ 6], 17, 0xa8304613);
     MD5_ROUND1(b, c, d, a, x[ 7], 22, 0xfd469501);
     MD5_ROUND1(a, b, c, d, x[ 8],  7, 0x698098d8);
     MD5_ROUND1(d, a, b, c, x[ 9], 12, 0x8b44f7af);
     MD5_ROUND1(c, d, a, b, x[10], 17, 0xffff5bb1);
     MD5_ROUND1(b, c, d, a, x[11], 22, 0x895cd7be);
     MD5_ROUND1(a, b, c, d, x[12],  7, 0x6b901122);
     MD5_ROUND1(d, a, b, c, x[13], 12, 0xfd987193);
     MD5_ROUND1(c, d, a, b, x[14], 17, 0xa679438e);
     MD5_ROUND1(b, c, d, a, x[15], 22, 0x49b40821);
 
     MD5_ROUND2(a, b, c, d, x[ 1],  5, 0xf61e2562);
     MD5_ROUND2(d, a, b, c, x[ 6],  9, 0xc040b340);
     MD5_ROUND2(c, d, a, b, x[11], 14, 0x265e5a51);
     MD5_ROUND2(b, c, d, a, x[ 0], 20, 0xe9b6c7aa);
     MD5_ROUND2(a, b, c, d, x[ 5],  5, 0xd62f105d);
     MD5_ROUND2(d, a, b, c, x[10],  9,  0x2441453);
     MD5_ROUND2(c, d, a, b, x[15], 14, 0xd8a1e681);
     MD5_ROUND2(b, c, d, a, x[ 4], 20, 0xe7d3fbc8);
     MD5_ROUND2(a, b, c, d, x[ 9],  5, 0x21e1cde6);
     MD5_ROUND2(d, a, b, c, x[14],  9, 0xc33707d6);
     MD5_ROUND2(c, d, a, b, x[ 3], 14, 0xf4d50d87);
     MD5_ROUND2(b, c, d, a, x[ 8], 20, 0x455a14ed);
     MD5_ROUND2(a, b, c, d, x[13],  5, 0xa9e3e905);
     MD5_ROUND2(d, a, b, c, x[ 2],  9, 0xfcefa3f8);
     MD5_ROUND2(c, d, a, b, x[ 7], 14, 0x676f02d9);
     MD5_ROUND2(b, c, d, a, x[12], 20, 0x8d2a4c8a);
 
     MD5_ROUND3(a, b, c, d, x[ 5],  4, 0xfffa3942);
     MD5_ROUND3(d, a, b, c, x[ 8], 11, 0x8771f681);
     MD5_ROUND3(c, d, a, b, x[11], 16, 0x6d9d6122);
     MD5_ROUND3(b, c, d, a, x[14], 23, 0xfde5380c);
     MD5_ROUND3(a, b, c, d, x[ 1],  4, 0xa4beea44);
     MD5_ROUND3(d, a, b, c, x[ 4], 11, 0x4bdecfa9);
     MD5_ROUND3(c, d, a, b, x[ 7], 16, 0xf6bb4b60);
     MD5_ROUND3(b, c, d, a, x[10], 23, 0xbebfbc70);
     MD5_ROUND3(a, b, c, d, x[13],  4, 0x289b7ec6);
     MD5_ROUND3(d, a, b, c, x[ 0], 11, 0xeaa127fa);
     MD5_ROUND3(c, d, a, b, x[ 3], 16, 0xd4ef3085);
     MD5_ROUND3(b, c, d, a, x[ 6], 23,  0x4881d05);
     MD5_ROUND3(a, b, c, d, x[ 9],  4, 0xd9d4d039);
     MD5_ROUND3(d, a, b, c, x[12], 11, 0xe6db99e5);
     MD5_ROUND3(c, d, a, b, x[15], 16, 0x1fa27cf8);
     MD5_ROUND3(b, c, d, a, x[ 2], 23, 0xc4ac5665);
 
     MD5_ROUND4(a, b, c, d, x[ 0],  6, 0xf4292244);
     MD5_ROUND4(d, a, b, c, x[ 7], 10, 0x432aff97);
     MD5_ROUND4(c, d, a, b, x[14], 15, 0xab9423a7);
     MD5_ROUND4(b, c, d, a, x[ 5], 21, 0xfc93a039);
     MD5_ROUND4(a, b, c, d, x[12],  6, 0x655b59c3);
     MD5_ROUND4(d, a, b, c, x[ 3], 10, 0x8f0ccc92);
     MD5_ROUND4(c, d, a, b, x[10], 15, 0xffeff47d);
     MD5_ROUND4(b, c, d, a, x[ 1], 21, 0x85845dd1);
     MD5_ROUND4(a, b, c, d, x[ 8],  6, 0x6fa87e4f);
     MD5_ROUND4(d, a, b, c, x[15], 10, 0xfe2ce6e0);
     MD5_ROUND4(c, d, a, b, x[ 6], 15, 0xa3014314);
     MD5_ROUND4(b, c, d, a, x[13], 21, 0x4e0811a1);
     MD5_ROUND4(a, b, c, d, x[ 4],  6, 0xf7537e82);
     MD5_ROUND4(d, a, b, c, x[11], 10, 0xbd3af235);
     MD5_ROUND4(c, d, a, b, x[ 2], 15, 0x2ad7d2bb);
     MD5_ROUND4(b, c, d, a, x[ 9], 21, 0xeb86d391);
 
     state[0] += a;
     state[1] += b;
     state[2] += c;
     state[3] += d;
 }
 
 
 /*!
 @brief      内部函数，处理数据的前面部分(>64字节的部分)，每次组成一个64字节的block就进行杂凑处理
 @param[out] ctx  算法的context，用于记录一些处理的上下文和结果
 @param[in]  buf  处理的数据，
 @param[in]  size 处理的数据长度
 */
 static void zen_md5_update(md5_ctx *ctx, const unsigned char *buf, size_t size)
 {
     //为什么不是=，因为在某些环境下，可以多次调用zen_md5_update，但这种情况，必须保证前面的调用，每次都没有unprocessed_
     ctx->length_ += size;
 
     //每个处理的块都是64字节
     while (size >= ZEN_MD5_BLOCK_SIZE)
     {
         zen_md5_process_block(ctx->hash_, reinterpret_cast<const DWORD *>(buf));
         buf  += ZEN_MD5_BLOCK_SIZE;
         size -= ZEN_MD5_BLOCK_SIZE;
     }
 
     ctx->unprocessed_ = size;
 }
 
 
 /*!
 @brief      内部函数，处理数据的末尾部分，我们要拼出最后1个（或者两个）要处理的BLOCK，加上0x80，加上长度进行处理
 @param[in]  ctx    算法的context，用于记录一些处理的上下文和结果
 @param[in]  buf    处理的数据
 @param[in]  size   处理buffer的长度
 @param[out] result 返回的结果，
 */
 static void zen_md5_final(md5_ctx *ctx, const unsigned char *buf, size_t size, unsigned char *result)
 {
     DWORD message[ZEN_MD5_BLOCK_SIZE / 4];
 
     //保存剩余的数据，我们要拼出最后1个（或者两个）要处理的块，前面的算法保证了，最后一个块肯定小于64个字节
     if (ctx->unprocessed_)
     {
         memcpy(message, buf + size - ctx->unprocessed_, static_cast<size_t>( ctx->unprocessed_));
     }
 
     //得到0x80要添加在的位置（在DWORD 数组中），
     DWORD index = ((DWORD)ctx->length_ & 63) >> 2;
     DWORD shift = ((DWORD)ctx->length_ & 3) * 8;
 
     //添加0x80进去，并且把余下的空间补充0
     message[index]   &= ~(0xFFFFFFFF << shift);
     message[index++] ^= 0x80 << shift;
 
     //如果这个block还无法处理，其后面的长度无法容纳长度64bit，那么先处理这个block
     if (index > 14)
     {
         while (index < 16)
         {
             message[index++] = 0;
         }
 
         zen_md5_process_block(ctx->hash_, message);
         index = 0;
     }
 
     //补0
     while (index < 14)
     {
         message[index++] = 0;
     }
 
     //保存长度，注意是bit位的长度,这个问题让我看着郁闷了半天，
     unsigned __int64 data_len = (ctx->length_) << 3;
 
     //注意MD5算法要求的64bit的长度是小头LITTLE-ENDIAN编码，注意下面的比较是!=
 #if ZEN_BYTES_ORDER != ZEN_LITTLE_ENDIAN
     data_len = ZEN_SWAP_UINT64(data_len);
 #endif
 
     message[14] = (DWORD) (data_len & 0x00000000FFFFFFFF);
     message[15] = (DWORD) ((data_len & 0xFFFFFFFF00000000ULL) >> 32);
 
     zen_md5_process_block(ctx->hash_, message);
 
     //注意结果是小头党的，在大头的世界要进行转换
 #if ZEN_BYTES_ORDER == ZEN_LITTLE_ENDIAN
     memcpy(result, &ctx->hash_, MD5_HASH_SIZE);
 #else
     swap_uint32_memcpy(result, &ctx->hash_, MD5_HASH_SIZE);
 #endif
 
 }
 
 
 //计算一个内存数据的MD5值
 unsigned char *md5(const unsigned char *buf,
                             size_t size,
                             unsigned char result[MD5_HASH_SIZE])
 {
     assert(result != NULL);
 
     md5_ctx ctx;
     zen_md5_init(&ctx);
     zen_md5_update(&ctx, buf, size);
     zen_md5_final(&ctx, buf, size, result);
     return result;
 }

void md5_begin(md5_ctx *ctx)
{
	zen_md5_init(ctx);
}

void md5_update(md5_ctx *ctx,const unsigned char *buf_,size_t size)
{
	unsigned char *buf=(unsigned char *)buf_;
	if (ctx->nRemains>0)
	{
		if (ctx->nRemains+size<ZEN_MD5_BLOCK_SIZE)
		{
			memcpy(ctx->remains+ctx->nRemains,buf,size);
			ctx->nRemains+=size;
			return;
		}

		int patch=ZEN_MD5_BLOCK_SIZE-ctx->nRemains;
		memcpy(ctx->remains+ctx->nRemains,buf,patch);
		zen_md5_update(ctx, ctx->remains,ZEN_MD5_BLOCK_SIZE);

		ctx->nRemains=0;

		buf+=patch;
		size-=patch;
	}

	ctx->nRemains=size%ZEN_MD5_BLOCK_SIZE;
	size-=ctx->nRemains;
	zen_md5_update(ctx, buf,size);

	if (ctx->nRemains>0)
		memcpy(ctx->remains,&buf[size],ctx->nRemains);
}

void md5_finalize(md5_ctx *ctx,unsigned char result[MD5_HASH_SIZE])
{
	zen_md5_final(ctx,ctx->remains,ctx->nRemains,result);
}

 
 
 
 
 //================================================================================================
 //SHA1的算法
 
 //每次处理的BLOCK的大小
 static const size_t ZEN_SHA1_BLOCK_SIZE = 64;
 
 //SHA1算法的上下文，保存一些状态，中间数据，结果
 typedef struct sha1_ctx
 {
 
     //处理的数据的长度
     unsigned __int64 length_;
     //还没有处理的数据长度
     unsigned __int64 unprocessed_;
     /* 160-bit algorithm internal hashing state */
     DWORD hash_[5];
 } sha1_ctx;
 
 //内部函数，SHA1算法的上下文的初始化
 static void zen_sha1_init(sha1_ctx *ctx)
 {
     ctx->length_ = 0;
     ctx->unprocessed_ = 0;
     // 初始化算法的几个常量，魔术数
     ctx->hash_[0] = 0x67452301;
     ctx->hash_[1] = 0xefcdab89;
     ctx->hash_[2] = 0x98badcfe;
     ctx->hash_[3] = 0x10325476;
     ctx->hash_[4] = 0xc3d2e1f0;
 }
 
 
 /*!
 @brief      内部函数，对一个64bit内存块进行摘要(杂凑)处理，
 @param      hash  存放计算hash结果的的数组
 @param      block 要计算的处理得内存块
 */
 static void zen_sha1_process_block(DWORD hash[5],
                                    const DWORD block[ZEN_SHA1_BLOCK_SIZE / 4])
 {
     size_t        t;
     DWORD      wblock[80];
     register DWORD      a, b, c, d, e, temp;
 
     //SHA1算法处理的内部数据要求是大头党的，在小头的环境转换
 #if ZEN_BYTES_ORDER == ZEN_LITTLE_ENDIAN
     swap_uint32_memcpy(wblock, block, ZEN_SHA1_BLOCK_SIZE);
 #else
     ::memcpy(wblock, block, ZEN_SHA1_BLOCK_SIZE);
 #endif
 
     //处理
     for (t = 16; t < 80; t++)
     {
         wblock[t] = ROTL32(wblock[t - 3] ^ wblock[t - 8] ^ wblock[t - 14] ^ wblock[t - 16], 1);
     }
 
     a = hash[0];
     b = hash[1];
     c = hash[2];
     d = hash[3];
     e = hash[4];
 
     for (t = 0; t < 20; t++)
     {
         /* the following is faster than ((B & C) | ((~B) & D)) */
         temp =  ROTL32(a, 5) + (((c ^ d) & b) ^ d)
                 + e + wblock[t] + 0x5A827999;
         e = d;
         d = c;
         c = ROTL32(b, 30);
         b = a;
         a = temp;
     }
 
     for (t = 20; t < 40; t++)
     {
         temp = ROTL32(a, 5) + (b ^ c ^ d) + e + wblock[t] + 0x6ED9EBA1;
         e = d;
         d = c;
         c = ROTL32(b, 30);
         b = a;
         a = temp;
     }
 
     for (t = 40; t < 60; t++)
     {
         temp = ROTL32(a, 5) + ((b & c) | (b & d) | (c & d))
                + e + wblock[t] + 0x8F1BBCDC;
         e = d;
         d = c;
         c = ROTL32(b, 30);
         b = a;
         a = temp;
     }
 
     for (t = 60; t < 80; t++)
     {
         temp = ROTL32(a, 5) + (b ^ c ^ d) + e + wblock[t] + 0xCA62C1D6;
         e = d;
         d = c;
         c = ROTL32(b, 30);
         b = a;
         a = temp;
     }
 
     hash[0] += a;
     hash[1] += b;
     hash[2] += c;
     hash[3] += d;
     hash[4] += e;
 }
 
 
 /*!
 @brief      内部函数，处理数据的前面部分(>64字节的部分)，每次组成一个64字节的block就进行杂凑处理
 @param      ctx  算法的上下文，记录中间数据，结果等
 @param      msg  要进行计算的数据buffer
 @param      size 长度
 */
 static void zen_sha1_update(sha1_ctx *ctx,
                             const unsigned char *buf, 
                             size_t size)
 {
     //为了让zen_sha1_update可以多次进入，长度可以累计
     ctx->length_ += size;
 
     //每个处理的块都是64字节
     while (size >= ZEN_SHA1_BLOCK_SIZE)
     {
         zen_sha1_process_block(ctx->hash_, reinterpret_cast<const DWORD *>(buf));
         buf  += ZEN_SHA1_BLOCK_SIZE;
         size -= ZEN_SHA1_BLOCK_SIZE;
     }
 
     ctx->unprocessed_ = size;
 }
 
 
 /*!
 @brief      内部函数，处理数据的最后部分，添加0x80,补0，增加长度信息
 @param      ctx    算法的上下文，记录中间数据，结果等
 @param      msg    要进行计算的数据buffer
 @param      result 返回的结果
 */
 static void zen_sha1_final(sha1_ctx *ctx, 
                            const unsigned char *msg,
                            size_t size, 
                            unsigned char *result)
 {
 
     DWORD message[ZEN_SHA1_BLOCK_SIZE / 4];
 
     //保存剩余的数据，我们要拼出最后1个（或者两个）要处理的块，前面的算法保证了，最后一个块肯定小于64个字节
     if (ctx->unprocessed_)
     {
         memcpy(message, msg + size - ctx->unprocessed_, static_cast<size_t>( ctx->unprocessed_));
     }
 
     //得到0x80要添加在的位置（在DWORD 数组中），
     DWORD index = ((DWORD)ctx->length_ & 63) >> 2;
     DWORD shift = ((DWORD)ctx->length_ & 3) * 8;
 
     //添加0x80进去，并且把余下的空间补充0
     message[index]   &= ~(0xFFFFFFFF << shift);
     message[index++] ^= 0x80 << shift;
 
     //如果这个block还无法处理，其后面的长度无法容纳长度64bit，那么先处理这个block
     if (index > 14)
     {
         while (index < 16)
         {
             message[index++] = 0;
         }
 
         zen_sha1_process_block(ctx->hash_, message);
         index = 0;
     }
 
     //补0
     while (index < 14)
     {
         message[index++] = 0;
     }
 
     //保存长度，注意是bit位的长度,这个问题让我看着郁闷了半天，
     unsigned __int64 data_len = (ctx->length_) << 3;
 
     //注意SHA1算法要求的64bit的长度是大头BIG-ENDIAN，在小头的世界要进行转换
 #if ZEN_BYTES_ORDER == ZEN_LITTLE_ENDIAN
     data_len = ZEN_SWAP_UINT64(data_len);
 #endif
 
     message[14] = (DWORD) (data_len & 0x00000000FFFFFFFF);
     message[15] = (DWORD) ((data_len & 0xFFFFFFFF00000000ULL) >> 32);
 
     zen_sha1_process_block(ctx->hash_, message);
 
     //注意结果是大头党的，在小头的世界要进行转换
 #if ZEN_BYTES_ORDER == ZEN_LITTLE_ENDIAN
     swap_uint32_memcpy(result, &ctx->hash_, SHA1_HASH_SIZE);
 #else
     memcpy(result, &ctx->hash_, SHA1_HASH_SIZE);
 #endif
 }
 
 
 
 //计算一个内存数据的SHA1值
 unsigned char *sha1(const unsigned char *msg,
                              size_t size,
                              unsigned char result[SHA1_HASH_SIZE])
 {
     assert(result != NULL);
 
     sha1_ctx ctx;
     zen_sha1_init(&ctx);
     zen_sha1_update(&ctx, msg, size);
     zen_sha1_final(&ctx, msg, size, result);
     return result;
 }
 
//  int main(int /*argc*/, char * /*argv*/[])
 //  {
 //  
 //      int ret = 0;
 //      static unsigned char test_buf[7][81] =
 //      {
 //          { "" }, 
 //          { "a" },
 //          { "abc" },
 //          { "message digest" },
 //          { "abcdefghijklmnopqrstuvwxyz" },
 //          { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" },
 //          { "12345678901234567890123456789012345678901234567890123456789012345678901234567890" }
 //      };
 //  
 //      static const size_t test_buflen[7] =
 //      {
 //          0, 1, 3, 14, 26, 62, 80
 //      };
 //  
 //      static const unsigned char md5_test_sum[7][16] =
 //      {
 //          { 0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,  0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E },
 //          { 0x0C, 0xC1, 0x75, 0xB9, 0xC0, 0xF1, 0xB6, 0xA8,  0x31, 0xC3, 0x99, 0xE2, 0x69, 0x77, 0x26, 0x61 },
 //          { 0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0,  0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72 },
 //          { 0xF9, 0x6B, 0x69, 0x7D, 0x7C, 0xB7, 0x93, 0x8D,  0x52, 0x5A, 0x2F, 0x31, 0xAA, 0xF1, 0x61, 0xD0 },
 //          { 0xC3, 0xFC, 0xD3, 0xD7, 0x61, 0x92, 0xE4, 0x00,  0x7D, 0xFB, 0x49, 0x6C, 0xCA, 0x67, 0xE1, 0x3B },
 //          { 0xD1, 0x74, 0xAB, 0x98, 0xD2, 0x77, 0xD9, 0xF5,  0xA5, 0x61, 0x1C, 0x2C, 0x9F, 0x41, 0x9D, 0x9F },
 //          { 0x57, 0xED, 0xF4, 0xA2, 0x2B, 0xE3, 0xC9, 0x55,  0xAC, 0x49, 0xDA, 0x2E, 0x21, 0x07, 0xB6, 0x7A }
 //      };
 //      unsigned char result[32] ={0};
 //  
 //      for(size_t i=0;i<7;++i)
 //      {
 //          md5(test_buf[i],test_buflen[i],result);
 //          ret = memcmp(result,md5_test_sum[i],16);
 //          if (ret != 0)
 //          {
 //              assert(false);
 //          }
 //      }
 //  
 //      static const unsigned char sha1_test_sum[7][20] =
 //      {
 //          { 0xda,0x39,0xa3,0xee,0x5e,0x6b,0x4b,0x0d,0x32,0x55,0xbf,0xef,0x95,0x60,0x18,0x90,0xaf,0xd8,0x07,0x09 },
 //          { 0x86,0xf7,0xe4,0x37,0xfa,0xa5,0xa7,0xfc,0xe1,0x5d,0x1d,0xdc,0xb9,0xea,0xea,0xea,0x37,0x76,0x67,0xb8 },
 //          { 0xa9,0x99,0x3e,0x36,0x47,0x06,0x81,0x6a,0xba,0x3e,0x25,0x71,0x78,0x50,0xc2,0x6c,0x9c,0xd0,0xd8,0x9d },
 //          { 0xc1,0x22,0x52,0xce,0xda,0x8b,0xe8,0x99,0x4d,0x5f,0xa0,0x29,0x0a,0x47,0x23,0x1c,0x1d,0x16,0xaa,0xe3 },
 //          { 0x32,0xd1,0x0c,0x7b,0x8c,0xf9,0x65,0x70,0xca,0x04,0xce,0x37,0xf2,0xa1,0x9d,0x84,0x24,0x0d,0x3a,0x89 },
 //          { 0x76,0x1c,0x45,0x7b,0xf7,0x3b,0x14,0xd2,0x7e,0x9e,0x92,0x65,0xc4,0x6f,0x4b,0x4d,0xda,0x11,0xf9,0x40 },
 //          { 0x50,0xab,0xf5,0x70,0x6a,0x15,0x09,0x90,0xa0,0x8b,0x2c,0x5e,0xa4,0x0f,0xa0,0xe5,0x85,0x55,0x47,0x32 },
 //      };
 //      for(size_t i=0;i<7;++i)
 //      {
 //          ZEN_LIB::sha1(test_buf[i],test_buflen[i],result);
 //          ret = memcmp(result,sha1_test_sum[i],20);
 //          if (ret != 0)
 //          {
 //              assert(false);
 //          }
 //      }
 //      return 0;
 //  }