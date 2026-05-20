
#pragma once
#include <mutex>
#include <condition_variable>


class ReadWriteLock
{
public:
	ReadWriteLock() : _readers(0), _writer(false) {}

	// ������
	void ReadLock() const
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_readCV.wait(lock, [this] { return !_writer; });
		++_readers;
	}

	// ������
	void ReadUnlock() const
	{
		std::unique_lock<std::mutex> lock(_mutex);
		--_readers;
		if (_readers == 0)
		{
			_writeCV.notify_one();
		}
	}

	// д����
	void WriteLock()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_writeCV.wait(lock, [this] { return !_writer && _readers == 0; });
		_writer = true;
	}

	// д����
	void WriteUnlock()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_writer = false;
		_readCV.notify_all();  // �������еȴ��Ķ���
	}

private:
	mutable std::mutex _mutex;
	mutable std::condition_variable _readCV;
	mutable std::condition_variable _writeCV;
	mutable int _readers;
	bool _writer;
};

class ReadLockGuard
{
public:
	ReadLockGuard(const ReadWriteLock& lock) : _lock(lock)
	{
		_lock.ReadLock();
	}

	~ReadLockGuard()
	{
		_lock.ReadUnlock();
	}

private:
	const ReadWriteLock& _lock;
};

class WriteLockGuard
{
public:
	WriteLockGuard(ReadWriteLock& lock) : _lock(lock)
	{
		_lock.WriteLock();
	}

	~WriteLockGuard()
	{
		_lock.WriteUnlock();
	}

private:
	ReadWriteLock& _lock;
};

