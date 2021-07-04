#pragma once
#include <Siv3D.hpp>
#include "Database.hpp"
#include "ImageReader.hpp"

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

		bool createTable(const String& arguments)
		{
			bool result = true;

			if (!exists())
			{
				char* errMsg;
				const String exec = U"create table if not exists {0} ({1});"_fmt(name, arguments);

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
			U"id integer primary key autoincrement unique, "
			U"width integer not null, "
			U"height integer not null, "
			U"digest text not null"
			U"data blob not null") {}
	};

	class ThumbTable : public Table
	{
	public:
		ThumbTable() : Table(
			U"thumb_t",
			U"imageid integer unique foreign_key references image_t(id) not null, "
			U"width integer not null,"
			U"height integer not null,"
			U"digest text not null,"
			U"data blob not null") {}
	};

	class TagTable : public Table
	{
	public:
		TagTable() : Table(
			U"tag_t",
			U"id integer primary key autoincrement unique not null, "
			U"name text unique not null") {}
	};

	class TagAssignTable : public Table
	{
	public:
		TagAssignTable() : Table(
			U"tag_asign_t",
			U"imageid integer foreign_key references image_t(id) not null, "
			U"tagid integer foreign_key references tag_t(id) not null") {}
	};

	class ImageToTagsTable : public Table
	{
	public:
		ImageToTagsTable() : Table(
			U"image_tags_t",
			U"imageid integer foreign_key references image_t(id) not null, "
			U"tagid integer foreign_key references tag_t(id) not null") {}
	};

	ImageTable* image_t = nullptr;
	ThumbTable* thumb_t = nullptr;
	TagTable* tag_t = nullptr;
	ImageToTagsTable* image_tags_t = nullptr;

	struct ImagePack
	{
		const int64 id;
		const Image source;
		const Image thumb;
		const std::string source_digest;
		const std::string thumb_digest;
	};

	const ImagePack insert_image(sqlite3* connection, const Image& source, bool useTransaction = true)
	{
		int64 id;
		const double wh = source.width() > source.height() ? (double)source.width() : (double)source.height();
		const double scaling = 128.0 / wh;
		
		// retval.source = source;
		const Image thumb = source.scaled(scaling, s3d::Interpolation::Area);
		const std::string source_digest = Unicode::NarrowAscii(image::get_digest(source));
		const std::string thumb_digest = Unicode::NarrowAscii(image::get_digest(thumb));
		
		if (useTransaction)
		{
			sqlite3_exec(connection, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		}
		{
			sqlite3_stmt* statement;

			sqlite3_prepare_v2(connection, "select max(id) from image_t limit 1;", -1, &statement, nullptr);
			sqlite3_step(statement);
			id = sqlite3_column_int64(statement, 0) + 1;
			sqlite3_finalize(statement);
		}
		ImagePack retval{ 
			id, std::move(source), std::move(thumb), 
			std::move(source_digest), std::move(thumb_digest) 
		};
		{
			sqlite3_stmt* statement;
			sqlite3_prepare_v2(
				connection,
				"insert all"
				"  into image_t(id, width, height, digest, data) values(:id, :iw, :ih, :idigest, :idata)"
				"  into thumb_t(imageid, width, height, digest, data) values (:id, :tw, :th, :tdigest, :tdata);",
				-1, &statement, nullptr);
			const int s_id = sqlite3_bind_parameter_index(statement, ":id");
			const int s_iw = sqlite3_bind_parameter_index(statement, ":iw");
			const int s_ih = sqlite3_bind_parameter_index(statement, ":ih");
			const int s_idigest = sqlite3_bind_parameter_index(statement, ":idigest");
			const int s_idata = sqlite3_bind_parameter_index(statement, ":idata");
			const int s_tw = sqlite3_bind_parameter_index(statement, ":tw");
			const int s_th = sqlite3_bind_parameter_index(statement, ":th");
			const int s_tdigest = sqlite3_bind_parameter_index(statement, ":tdigest");
			const int s_tdata = sqlite3_bind_parameter_index(statement, ":tdata");

			sqlite3_bind_int64(statement, s_id, retval.id);
			sqlite3_bind_int(statement, s_iw, source.width());
			sqlite3_bind_int(statement, s_ih, source.height());
			sqlite3_bind_text(statement, s_idigest, retval.source_digest.c_str(), retval.source_digest.size(), nullptr);
			sqlite3_bind_blob(statement, s_idata, (void*)source.data(), source.size_bytes(), nullptr);
			sqlite3_bind_int(statement, s_tw, retval.thumb.width());
			sqlite3_bind_int(statement, s_th, retval.thumb.height());
			sqlite3_bind_text(statement, s_tdigest, retval.thumb_digest.c_str(), retval.thumb_digest.size(), nullptr);
			sqlite3_bind_blob(statement, s_tdata, (void*)retval.thumb.data(), retval.thumb.size_bytes(), nullptr);
			sqlite3_step(statement);
			sqlite3_finalize(statement);
		}
		if (useTransaction)
		{
			sqlite3_exec(connection, "END TRANSACTION;", NULL, NULL, NULL);
		}

		return retval;
	}

	const Array<ImagePack> insert_images(sqlite3* connection, const Array<Image> imgs)
	{
		Array<ImagePack> retval;
		sqlite3_exec(connection, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		for (const auto& img : imgs)
			retval.emplace_back(insert_image(connection, img, false));
		sqlite3_exec(connection, "END TRANSACTION;", NULL, NULL, NULL);

		return retval;
	}

	void initializeTables()
	{
		image_t = new ImageTable();
		thumb_t = new ThumbTable();
		tag_t = new TagTable();
		image_tags_t = new ImageToTagsTable();
	}

	void finalizeTables()
	{
		delete image_t;
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