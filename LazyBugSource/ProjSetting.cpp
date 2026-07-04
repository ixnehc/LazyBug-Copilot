#include "stdh.h"

#include "ProjSetting.h"
#include "datapacket/DataPacket.h"

#include "Utils_File.h"

//////////////////////////////////////////////////////////////////////////
//CProjSettingLib

void CProjSettingLib::Init(const char* fullPath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_fullPath = fullPath ? fullPath : "";
    _isDirty = false; // 初始化时重置脏位标志
}

void CProjSettingLib::Clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 释放所有ProjSetting对象
    for (auto& pair : _uniqueSettings)
    {
        delete pair.second;
    }
    _uniqueSettings.clear();
    
    m_fullPath.clear();
    _isDirty = false; // 清理后重置脏位标志
}

void CProjSettingLib::Load()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_fullPath.empty())
        return;
        
    // 使用STL的ifstream检查文件是否存在
    std::ifstream file;
	Utils::OpenIFStream(file, m_fullPath.c_str());

    if (!file.is_open())
        return;
        
    try
    {
        // 清理现有数据
        for (auto& pair : _uniqueSettings)
        {
            delete pair.second;
        }
        _uniqueSettings.clear();
        
        // 读取文件大小
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (fileSize == 0)
        {
            file.close();
            return;
        }
        
        // 读取文件内容到缓冲区
        std::vector<BYTE> buffer(fileSize);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();
        
        // 创建DataPacket并设置数据缓冲区
        CDataPacket dp;
        dp.SetDataBufferPointer(buffer.data());
        
        // 从DataPacket反序列化
        DeserializeFromDataPacket(dp);
        
        // 加载成功后重置脏位标志
        _isDirty = false;
    }
    catch (...)
    {
        // 加载失败时清理
        for (auto& pair : _uniqueSettings)
        {
            delete pair.second;
        }
        _uniqueSettings.clear();
        _isDirty = false; // 加载失败也重置脏位标志
        
        file.close();
    }
}

void CProjSettingLib::Save()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_fullPath.empty())
        return;
        
    try
    {
        std::vector<BYTE> buffer;
        
        // 使用DP_BeginSave/DP_EndSave宏来处理DataPacket
        DP_BeginSave(dp, buffer)
        {
            SerializeToDataPacket(dp);
        }
        DP_EndSave()
        
        // 使用STL的ofstream写入文件
        std::ofstream file;
		Utils::OpenOFStream(file, m_fullPath.c_str());

        if (!file.is_open())
            return;
            
        if (!buffer.empty())
        {
            file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        }
        file.close();
        
        // 保存成功后重置脏位标志
        _isDirty = false;
    }
    catch (...)
    {
        // 保存失败，不重置脏位标志，保持数据需要保存的状态
    }
}

void CProjSettingLib::SerializeToDataPacket(CDataPacket& dp) const
{
    // 写入版本号（版本2使用WUID）
    dp.Data_WriteSimple<int>(2);
    
    // 写入设置数量
    dp.Data_WriteSimple<int>(static_cast<int>(_uniqueSettings.size()));
    
    // 写入每个设置
    for (const auto& pair : _uniqueSettings)
    {
        ProjSettingHandle handle = pair.first;
        const ProjSetting* pSetting = pair.second;
        
        // 写入句柄
        dp.Data_WriteSimple<ProjSettingHandle>(handle);
        
        // 写入additionalIncludeFullPathes向量
        dp.Data_WriteSimple<int>(static_cast<int>(pSetting->additionalIncludeFullPathes.size()));
        for (const std::string& path : pSetting->additionalIncludeFullPathes)
        {
            dp.Data_WriteString(path);
        }
        
        // 写入预编译头文件路径
        dp.Data_WriteString(pSetting->lowerCasedPchFullPath);
        dp.Data_WriteString(pSetting->lowerCasedPchOutputFullPath);
    }
}

void CProjSettingLib::DeserializeFromDataPacket(CDataPacket& dp)
{
    // 读取版本号
    int version = dp.Data_ReadSimple<int>();
    
    if (version == 2) // 使用WUID版本
    {
        // 读取设置数量
        int count = dp.Data_ReadSimple<int>();
        
        // 读取每个设置
        for (int i = 0; i < count; i++)
        {
            // 读取句柄
            ProjSettingHandle handle = dp.Data_ReadSimple<ProjSettingHandle>();
            
            ProjSetting* pSetting = new ProjSetting();
            
            // 读取additionalIncludeFullPathes向量
            int pathCount = dp.Data_ReadSimple<int>();
            pSetting->additionalIncludeFullPathes.resize(pathCount);
            
            for (int j = 0; j < pathCount; j++)
            {
                std::string path;
                dp.Data_ReadString(path);
                pSetting->additionalIncludeFullPathes[j] = path;
            }
            
            // 读取预编译头文件路径
            dp.Data_ReadString(pSetting->lowerCasedPchFullPath);
            dp.Data_ReadString(pSetting->lowerCasedPchOutputFullPath);
            
            _uniqueSettings[handle] = pSetting;
        }
    }
}

bool CProjSettingLib::IsDirty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _isDirty;
}

ProjSettingHandle CProjSettingLib::Add(const ProjSetting& setting)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 检查是否已经存在相同的设置，避免重复
    for (const auto& pair : _uniqueSettings)
    {
        const ProjSetting* existing = pair.second;
        if (existing->Equals(setting))
        {
            // 返回现有的句柄，不设置脏位标志（没有新增数据）
            return pair.first;
        }
    }
    
    // 创建新的设置对象
    ProjSetting* pNewSetting = new ProjSetting(setting);
    
    // 使用WUID生成唯一句柄
    ProjSettingHandle newHandle = GenWUID();
    
    // 添加到映射表
    _uniqueSettings[newHandle] = pNewSetting;
    
    // 设置脏位标志，表示数据已被修改
    _isDirty = true;
    
    return newHandle;
}

const ProjSetting* CProjSettingLib::Get(ProjSettingHandle handle)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = _uniqueSettings.find(handle);
    if (it != _uniqueSettings.end())
    {
        return it->second;
    }
    
    return nullptr;
}


