#include "stdh.h"
#include "encrypt.h"

unsigned int encrypt(unsigned int value)
{
	// 使用32位质数常量
	const unsigned int PRIME1 = 0x9E3779B1;  // 2654435761
	const unsigned int PRIME2 = 0x85EBCA77;  // 2246822519
	const unsigned int PRIME3 = 0xC2B2AE3D;  // 3266489917

	// 异或和位移操作确保可逆性
	unsigned int encrypted = value;

	// 混淆操作 1
	encrypted ^= PRIME1;
	encrypted *= PRIME2;

	// 位移操作增加扩散性
	encrypted = (encrypted << 13) | (encrypted >> 19);

	// 混淆操作 2
	encrypted ^= PRIME3;
	encrypted *= PRIME1;

	// 最终位移
	encrypted = (encrypted << 7) | (encrypted >> 25);

	return encrypted;
}

// 计算32位无符号整数的乘法逆元
// 注意：仅当a和2^32互质时才有逆元，这里PRIME1和PRIME2都是奇数，所以满足条件
unsigned int findMultiplicativeInverse(unsigned int a)
{
	// 使用扩展欧几里得算法的变种来计算乘法逆元
	unsigned int a_inv = a;

	// 应用牛顿迭代法计算逆元
	// a * a_inv ≡ 1 (mod 2^32)
	// 迭代公式: a_inv = a_inv * (2 - a * a_inv)
	a_inv *= 2 - a * a_inv;
	a_inv *= 2 - a * a_inv;
	a_inv *= 2 - a * a_inv;
	a_inv *= 2 - a * a_inv;
	a_inv *= 2 - a * a_inv;

	return a_inv;
}

unsigned int decrypt(unsigned int encrypted)
{
	// 使用相同的常量
	const unsigned int PRIME1 = 0x9E3779B1;
	const unsigned int PRIME2 = 0x85EBCA77;
	const unsigned int PRIME3 = 0xC2B2AE3D;

	// 反向执行所有操作
	unsigned int decrypted = encrypted;

	// 反转最终位移
	decrypted = (decrypted >> 7) | (decrypted << 25);

	// 反转混淆操作 2
	// 注意：乘法需要使用乘法逆元
	decrypted *= findMultiplicativeInverse(PRIME1);
	decrypted ^= PRIME3;

	// 反转位移操作
	decrypted = (decrypted >> 13) | (decrypted << 19);

	// 反转混淆操作 1
	decrypted *= findMultiplicativeInverse(PRIME2);
	decrypted ^= PRIME1;

	return decrypted;
}

