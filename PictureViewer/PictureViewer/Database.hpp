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

	const size_t sizeofString(const std::string& str)
	{
		return str.cend() - str.cbegin();
	}

	const bool existsTable(const String& tablename)
	{
		const String exec = U"select count(*) from sqlite_master where type = 'table' AND name=?;";
		sqlite3_stmt* statement;

		const std::string tablename_utf8 = tablename.toUTF8();

		auto ret = sqlite3_prepare(connection, exec.toUTF8().c_str(), (int)exec.toUTF8().size(), &statement, NULL);
		bool result = false;

		if (ret == SQLITE_OK && statement)
		{
			sqlite3_bind_text(statement, 1, tablename_utf8.c_str(), (int)sizeofString(tablename_utf8), SQLITE_STATIC);

			int col_count = sqlite3_column_count(statement);
			if (col_count == 1)
			{
				sqlite3_step(statement);
				int exists = sqlite3_column_int(statement, 1);

				if (exists == 1)
					result = true;
			}
			
			sqlite3_finalize(statement);
		}
		return result;
	}

	class Table
	{
	public:
		const String name;

	private:
		bool createTable(const String& brack)
		{
			bool result = true;

			if (!exists())
			{
				char* errMsg;
				const String exec = U"create table if not exists {0} ({1});"_fmt(name, brack);

				auto ret = sqlite3_exec(connection, exec.toUTF8().c_str(), NULL, NULL, &errMsg);
				if (ret != SQLITE_OK)
				{
					Logger << Unicode::FromUTF8(errMsg) << U"\n";
					sqlite3_free(errMsg);
					result = false;
				}
			}
			return result;
		}

	public:
		Table(const String& _name, const String& create_table_in_brack) : name(_name)
		{
			createTable(create_table_in_brack);
		}

		const bool exists() const
		{
			return existsTable(name);
		}
	};

	class ImageTable : public Table
	{
	public:
		ImageTable() : Table(
			U"image_t", 
			U"id integer primary key autoincrement unique, data blob") {}
	};

	class InfoTable : public Table
	{
	public:
		InfoTable() : Table(
			U"info_t", 
			U"imageid integer unique, width integer, height integer, hashcode text") {}
	};

	class ThumbnailTable : public Table
	{
	public:
		ThumbnailTable() : Table(
			U"thumb_t", 
			U"imageid integer unique, width integer, height integer, data blob") {}
	};

	class TagTable : public Table
	{
	public:
		TagTable() : Table(
			U"tag_t", 
			U"id integer primary key autoincrement unique, name text unique") {}
	};

	class ImageToTagsTable : public Table
	{
	public:
		ImageToTagsTable() : Table(
			U"image_tags_t", 
			U"imageid integer, tagid integer") {}
	};

	void initializeTables()
	{
		ImageTable image_t;
		InfoTable info_t;
		ThumbnailTable thumb_t;
		TagTable tag_t;
		ImageToTagsTable image_tags_t;
	}

	void manuallyDropTables()
	{
		char* errMsg;
		auto ret = sqlite3_exec(connection,
			"drop table image_t;"
			"drop table info_t;"
			"drop table thumb_t;"
			"drop table tag_t;"
			"drop table image_tags_t;",
			NULL, NULL, &errMsg
		);

		if (ret != SQLITE_OK)
		{
			Logger << Unicode::FromUTF8(errMsg) << U"\n";
			sqlite3_free(errMsg);
		}
	}
}