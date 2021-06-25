#pragma once
#include <Siv3D.hpp>
#include "sqlite3.h"

namespace db
{
	sqlite3* connection = nullptr;
	String path = U"./database.db";
	bool isOpenDB = false;

	const bool connectDB()
	{
		if (isOpenDB == false && sqlite3_open(path.toUTF8().c_str(), &connection) == SQLITE_OK)
		{
			isOpenDB = true;
		}
		return isOpenDB;
	}

	const bool closeDB()
	{
		if (isOpenDB == true && sqlite3_close(connection) == SQLITE_OK)
		{
			isOpenDB = false;
		}
		return !isOpenDB;
	}
}