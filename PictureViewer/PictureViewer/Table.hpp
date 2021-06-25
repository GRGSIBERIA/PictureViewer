#pragma once
#include <Siv3D.hpp>
#include "Database.hpp"

namespace db
{
	class Table
	{
	public:
		const String name;

	private:
		const size_t sizeofString(const std::string& str)
		{
			return str.cend() - str.cbegin();
		}

		const bool existsTable()
		{
			const String exec = U"select count(*) from sqlite_master where type = 'table' AND name=?;";
			sqlite3_stmt* statement;

			const std::string tablename_utf8 = name.toUTF8();

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

		const bool exists()
		{
			return existsTable();
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

	class ThumbTable : public Table
	{
	public:
		ThumbTable() : Table(
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

	ImageTable* image_t = nullptr;
	InfoTable* info_t = nullptr;
	ThumbTable* thumb_t = nullptr;
	TagTable* tag_t = nullptr;
	ImageToTagsTable* image_tags_t = nullptr;

	void initializeTables()
	{
		image_t = new ImageTable();
		info_t = new InfoTable();
		thumb_t = new ThumbTable();
		tag_t = new TagTable();
		image_tags_t = new ImageToTagsTable();
	}

	void finalizeTables()
	{
		delete image_t;
		delete info_t;
		delete thumb_t;
		delete tag_t;
		delete image_tags_t;
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