#pragma once

#include "SolutionDB.h"

class CSolutionDBs
{
public:

	CSolutionDB* Obtain(const char* dbFolder, const char* slnPath = nullptr);

	void Update();

	void CloseAll();

public:

	std::unordered_map<std::string, CSolutionDB> _entries;

	mutable std::shared_mutex _mutex;

};