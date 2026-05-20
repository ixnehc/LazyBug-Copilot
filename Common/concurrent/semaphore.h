
#pragma once

struct Semaphore
{

	Semaphore(DWORD count=0);
	virtual ~Semaphore();

	/** The V operation of disktra semaphores */
	void Wait();
	/** The P operation of disktra semaphores */
	void Post();

	HANDLE handle;

};

