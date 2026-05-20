#pragma once


enum SscState
{
	SSC_UNKNOWN=-2,
	SSC_NOTCONTROLLED = -1, 
	SSC_NOTCHECKEDOUT = 0,
	SSC_CHECKEDOUT = 1,
	SSC_CHECKEDOUT_ME = 2,
};

enum SscFlags
{
	// Indicate whether the read-only flag should be used and assumed in the
	// user's local director
	SF_USERRONO = 1,
	SF_USERROYES = 2,

	// Indicate what to do if a Get would replace an existing writable file 
	// on the user's local drive: replace the file, skip the file, or merge 
	// into the file.
	SF_REPREPLACE = 128,
	SF_REPSKIP = 192,
	SF_REPMERGE = 256,

	// Indicate whether Visual SourceSafe should keep the file checked out 
	// after adding or checking it in.
	SF_KEEPNO = 65536,
	SF_KEEPYES = 131072,

	// Indicates whether a checkout should be exclusive. If you do not set 
	// this flag, the Checkout may be exclusive anyway due to the administrator's 
	// setting or the file type. But if you do set this flag, the Checkout will 
	// always be exclusive.
	SF_CHKEXCLUSIVEYES = 268435456,
	SF_CHKEXCLUSIVENO = 536870912
};