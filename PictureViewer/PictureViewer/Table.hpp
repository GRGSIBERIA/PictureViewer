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

		const bool occurred_error_writes_message_to_log(const int ok, char* err_msg) const
		{
			bool result = true;
			if (ok != SQLITE_OK)
			{
				Logger << Unicode::FromUTF8(err_msg) << U"\n";
				sqlite3_free(err_msg);
				result = false;
			}
			return result;
		}

		const bool create_table(const String& arguments)
		{
			bool result = true;
			char* err_msg;
			
			auto ret = sqlite3_exec(connection, arguments.toUTF8().c_str(), NULL, NULL, &err_msg);
			result = occurred_error_writes_message_to_log(ret, err_msg);
			
			return result;
		}

		const bool create_index(const String& table_name, const String& index_sql)
		{
			char* err_msg;
			auto ret = sqlite3_exec(connection, index_sql.toUTF8().c_str(), NULL, NULL, &err_msg);

			return occurred_error_writes_message_to_log(ret, err_msg);
		}

	public:
		Table(const String& _name, const String& create_table_sql, const String& index_sql) : name(_name)
		{
			create_table(create_table_sql);

			if (index_sql != U"")
				create_index(_name, index_sql);
		}
	};

	class ImageTable : public Table
	{
	public:
		ImageTable() : Table(
			U"image_t",
			U"create table if not exists image_t ("
			U"	id integer primary key autoincrement unique, "
			U"	width integer not null, "
			U"	height integer not null, "
			U"	digest text not null,"
			U"	data blob not null);",
			U"create unique index if not exists digest_image_index on image_t(digest asc);") {}
	};

	class ThumbTable : public Table
	{
	public:
		ThumbTable() : Table(
			U"thumb_t",
			U"create table if not exists thumb_t ("
			U"	imageid integer unique not null references image_t(id) on update cascade on delete cascade, "
			U"	width integer not null,"
			U"	height integer not null,"
			U"	digest text not null,"
			U"	data blob not null);",
			U"create unique index if not exists imageid_thumb_index on thumb_t(imageid asc);"
			U"create unique index if not exists digest_thumb_index on thumb_t(digest asc);") {}
	};

	class TagTable : public Table
	{
	public:
		TagTable() : Table(
			U"tag_t",
			U"create table if not exists tag_t("
			U"	id integer primary key autoincrement unique not null, "
			U"	name text unique not null);",
			U"create index if not exists name_tag_index on tag_t(name asc);") {}
	};

	class TagAssignTable : public Table
	{
	public:
		TagAssignTable() : Table(
			U"tag_assign_t",
			U"create table if not exists tag_assign_t("
			U"	imageid integer not null references image_t(id) on update cascade on delete cascade, "
			U"	tagid integer not null references tag_t(id) on update cascade on delete cascade);",
			U"create unique index if not exists imageid_tagid_tag_assign_index on tag_assign_t(imageid, tagid);") {}
	};

	ImageTable* image_t = nullptr;
	ThumbTable* thumb_t = nullptr;
	TagTable* tag_t = nullptr;
	TagAssignTable* image_tags_t = nullptr;

	struct ImagePack
	{
		const int64 id;
		const Image source;
		const Image thumb;
		const std::string source_digest;
		const std::string thumb_digest;
	};

	const int64 get_unuse_id(sqlite3* connection)
	{
		sqlite3_stmt* statement;

		sqlite3_prepare_v2(connection, "select max(id) from image_t limit 1;", -1, &statement, nullptr);
		sqlite3_step(statement);
		int64 id = sqlite3_column_int64(statement, 0) + 1;
		sqlite3_finalize(statement);

		return id;
	}

	const ImagePack insert_image(sqlite3* connection, const Image& source, const int64 usually_id = -1, const bool use_transaction = true)
	{
		int64 id = usually_id;
		const double wh = source.width() > source.height() ? (double)source.width() : (double)source.height();
		const double scaling = 128.0 / wh;
		
		// retval.source = source;
		const Image thumb = source.scaled(scaling, s3d::Interpolation::Area);
		const std::string source_digest = Unicode::NarrowAscii(image::get_digest(source));
		const std::string thumb_digest = Unicode::NarrowAscii(image::get_digest(thumb));
		
		if (use_transaction)
		{
			sqlite3_exec(connection, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		}
		if (usually_id < 0)
		{
			id = get_unuse_id(connection);
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
				"  into thumb_t(imageid, width, height, digest, data) values (:id, :tw, :th, :tdigest, :tdata)"
				"  select * from dual;",
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
		if (use_transaction)
		{
			sqlite3_exec(connection, "END TRANSACTION;", NULL, NULL, NULL);
		}

		return retval;
	}

	const Array<ImagePack> insert_images(sqlite3* connection, const Array<Image> imgs)
	{
		Array<ImagePack> retval;
		auto usually_id = get_unuse_id(connection);
		sqlite3_exec(connection, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		for (const auto& img : imgs)
		{
			retval.emplace_back(insert_image(connection, img, usually_id, false));
			++usually_id;
		}
		sqlite3_exec(connection, "END TRANSACTION;", NULL, NULL, NULL);

		return retval;
	}

	void initialize_tables()
	{
		image_t = new ImageTable();
		thumb_t = new ThumbTable();
		tag_t = new TagTable();
		image_tags_t = new TagAssignTable();
	}

	void finalize_tables()
	{
		delete image_t;
		delete thumb_t;
		delete tag_t;
		delete image_tags_t;
	}

	void manually_drop_tables()
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