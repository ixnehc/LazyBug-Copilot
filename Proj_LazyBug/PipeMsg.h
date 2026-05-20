#pragma once

#include "datapacket/DataPacket.h"
#include <memory>
#include <future>
#include <chrono>

typedef int PipeMsgType;

struct PipeMsg
{
public:
	virtual ~PipeMsg() = default;

	virtual PipeMsgType GetType() const = 0;

	// 使用 CDataPacket 将消息内容序列化
	virtual void Save(CDataPacket& dp) const = 0;

	// 使用 CDataPacket 从中反序列化消息内容
	virtual void Load(CDataPacket& dp) = 0;
}; 


typedef std::unique_ptr<PipeMsg> PipeMsgPtr;


struct FuturePipeMsg
{
public:
    FuturePipeMsg() = default;                                   // 构造一个“无效”实例
    explicit FuturePipeMsg(std::future<PipeMsgPtr> future) : _future(std::move(future)), _valid(true) {}

    // 显式有效性查询
    bool IsValid() const { return _valid && _future.valid(); }

	template <class T>
	bool Fetch(T& msg)//非阻塞取走
	{
		if (!IsValid() || _future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
            return false;
        return _FetchInternal(msg);
	}

	template <class T>
	bool WaitAndFetch(T& msg)//阻塞取走
	{
		if (!IsValid()) return false;
        return _FetchInternal(msg);
	}

private:
	template <class T>
    bool _FetchInternal(T& msg)
    {
        try 
        {
            PipeMsgPtr responseMsg = _future.get();
            if (!responseMsg) return false;

            // Type safety check
            T defaultConstructedMsgForTypeCheck;
            if (responseMsg->GetType() != defaultConstructedMsgForTypeCheck.GetType())
            {
				msg = defaultConstructedMsgForTypeCheck;
                return true; // The received message type does not match the requested type T, set it to a default value
            }

            T* specificMsg = dynamic_cast<T*>(responseMsg.get());
            if (!specificMsg) return false;

            msg = *specificMsg;
            return true;
        }
        catch (const std::future_error&)
        {
            return false;
        }
    }
	std::future<PipeMsgPtr> _future;
    bool _valid = false;   // 额外标志：对象是否处于“有效”状态
};