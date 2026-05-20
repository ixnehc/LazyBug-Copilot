#pragma once

#include <string>
#include <vector>
#include <map>

struct ResData;

struct RDGroupItem
{
	//a full path(such as "C:\\ResData\\abc.1rs|sphere01.mesh")
	//could be a path to a res node,a res(could not be a path to folder or file)
	std::string path;
	ResData *resdata;//NOTE:if NULL,the path is for a res node

	RDGroupItem &operator=(const RDGroupItem&src)
	{
		path=src.path;
		resdata=src.resdata;
		return *this;
	}

};



class ResDataGroup:public std::vector<RDGroupItem>
{
public:
	~ResDataGroup()
	{
	}
	void Clean();
	void Erase(int i);
	void PushBack(const char *path,ResData *resdata);
	int Find(const char *pathToFind);//Not case sensitive
	BOOL Copy(ResDataGroup&src);
	BOOL Move(ResDataGroup&src);//move another map into me(src will be cleared)
	BOOL Merge(ResDataGroup&src);
	void Sort();
};

struct RDListItem
{
	//a full path(such as "C:\\ResData\\abc.1rs|sphere01.mesh")
	//could be a path to a folder,a file,a res node,a res
	std::string path;
	int type;
	RDListItem &operator=(const RDListItem&src)
	{
		path=src.path;
		type=src.type;
		return *this;
	}
};

class ResDataList:public std::vector<RDListItem>
{
public:
	void PushBack(const char *path,int type)
	{
		RDListItem t;
		t.path=path;
		t.type=type;
		push_back(t);
	}
	void FromRdg(ResDataGroup &rdg);
};

struct RDRenameItem
{
	std::string path;
	std::string nameNew;//to change the end name in path
	RDRenameItem &operator=(const RDRenameItem&src)
	{
		path=src.path;
		nameNew=nameNew;
		return *this;
	}
};

class ResDataRename:public std::vector<RDRenameItem>
{
public:
	void PushBack(const char *path,const char *name)
	{
		RDRenameItem t;
		t.path=path;
		t.nameNew=name;
		push_back(t);
	}
};

