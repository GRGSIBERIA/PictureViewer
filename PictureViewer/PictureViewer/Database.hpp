#pragma once
#include <Siv3D.hpp>
#include "sqlite3.h"

namespace db
{
	sqlite3* connection = nullptr;
	String path = U"./database.db";

	const int sizeofString(const std::string& str)
	{
		return str.cend() - str.cbegin();
	}

	const bool existsTable(const String& tablename)
	{
		const String exec = U"select count(*) from sqlite_master where type = 'table' AND name=?;";
		sqlite3_stmt* statement;

		const std::string tablename_utf8 = tablename.toUTF8();

		sqlite3_prepare(connection, exec.toUTF8().c_str(), -1, &statement, NULL);
		sqlite3_bind_text(statement, 1, tablename_utf8.c_str(), sizeofString(tablename_utf8), SQLITE_TRANSIENT);

		int col_count = sqlite3_column_count(statement);
		if (col_count == 1)
		{
			sqlite3_step(statement);
			int exists = sqlite3_column_int(statement, 1);

			if (exists == 1)
				return true;
		}
		return false;
	}

	void initializeTables()
	{
		{
			const String exec = U"";
		}
	}
}