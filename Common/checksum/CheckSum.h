// CheckSum.h: interface for the CCheckSum class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHECKSUM_H__A7A8646C_2393_47B9_8E6A_F80A6C8E1A8B__INCLUDED_)
#define AFX_CHECKSUM_H__A7A8646C_2393_47B9_8E6A_F80A6C8E1A8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern void gen_crc_table();

extern unsigned long update_crc(unsigned long crc_accum, char *data_blk_ptr,
						 int data_blk_size);
						 



#endif // !defined(AFX_CHECKSUM_H__A7A8646C_2393_47B9_8E6A_F80A6C8E1A8B__INCLUDED_)
