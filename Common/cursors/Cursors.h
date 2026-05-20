#pragma once

#include <unordered_map>


class IAssetSystem;
class IAssetShell;

class CCursors
{
public:

	CCursors();
	~CCursors();

public:
	BOOL IsInit()	{		return _shell?TRUE:FALSE;	}
	BOOL Init(IAssetSystem*pAS);
	void	Clear();

	HCURSOR GetActive();

private:
	IAssetShell *_shell;
	std::string _nameActive;//
	std::unordered_map<std::string,HCURSOR> _cursors;


};
