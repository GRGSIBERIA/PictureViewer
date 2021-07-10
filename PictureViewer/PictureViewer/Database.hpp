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
		auto ok = sqlite3_open(path.toUTF8().c_str(), &connection);
		if (isOpenDB == false && ok == SQLITE_OK)
		{
			isOpenDB = true;
			sqlite3_exec(connection, "pragma FOREIGN_KEYS=ON;", NULL, NULL, NULL);
		}
		else if (ok != SQLITE_OK)
		{
			Logger.writeln(U"OCCURRED TO FAIL TO OPEN DATABASE >> {}"_fmt(ok));
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

	const bool rollbackDB()
	{
		char* err_msg = nullptr;
		auto ok = sqlite3_exec(connection, "rollback;", NULL, NULL, &err_msg);
		if (ok != SQLITE_OK)
		{
			Logger.writeln(Unicode::FromUTF8(err_msg));
			sqlite3_free(err_msg);
		}
	}
}