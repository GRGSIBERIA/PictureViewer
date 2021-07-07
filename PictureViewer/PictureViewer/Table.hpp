﻿#pragma once
#include <Siv3D.hpp>
#include <sqlite3.h>
#include <Windows.h>
#include "Database.hpp"

namespace db
{
	/// <summary>
	/// 画像からハッシュ値を取得する
	/// </summary>
	/// <param name="source">元の画像</param>
	/// <returns>SHA256のダイジェスト</returns>
	const String get_digest(const Image& source)
	{
		DWORD cryptSize = 32;
		HCRYPTPROV prov;
		HCRYPTHASH hash;
		PBYTE digest;
		String retval = U"";

		CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
		CryptCreateHash(prov, CALG_SHA_256, 0, 0, &hash);
		CryptHashData(hash, (BYTE*)source.data(), (DWORD)source.size_bytes(), 0);

		digest = (BYTE*)malloc(cryptSize);
		CryptGetHashParam(hash, HP_HASHVAL, digest, &cryptSize, 0);

		std::string buf;
		buf.assign(digest, digest + cryptSize);
		retval = Unicode::WidenAscii(buf);

		free(digest);

		return retval;
	}

	/// <summary>
	/// SQLite3の戻り値がSQLITE_OKでないときに、エラー内容をログに出力する
	/// </summary>
	/// <param name="ok">SQLite3関数の戻り値</param>
	/// <param name="err_msg">エラー文へのポインタ</param>
	/// <returns>SQLITE_OK == ok</returns>
	const bool occured_error_writes_message_to_log(const int ok, char* err_msg)
	{
		bool result = true;
		if (ok != SQLITE_OK)
		{
			Logger << Unicode::WidenAscii(err_msg);
			sqlite3_free(err_msg);
			result = false;
		}
		return result;
	}

	/// <summary>
	/// テーブルを初期化する
	/// </summary>
	/// <returns>SQLITE_OK == returns</returns>
	const bool create_tables(sqlite3* db_connection)
	{
		char* err_msg;
		auto text = TextReader(s3d::Resource(U"SQL/initialize_tables.sql"));
		const String sql = text.readAll();

		auto ok = sqlite3_exec(db_connection, sql.toUTF8().c_str(), NULL, NULL, &err_msg);
		return occured_error_writes_message_to_log(ok, err_msg);
	}

	/// <summary>
	/// 存在するテーブルを削除する
	/// </summary>
	/// <returns>SQLITE_OK == returns</returns>
	const bool manually_drop_tables(sqlite3* db_connection)
	{
		char* err_msg;
		auto text = TextReader(s3d::Resource(U"SQL/manually_drop.sql"));
		const String sql = text.readAll();

		auto ok = sqlite3_exec(db_connection, sql.toUTF8().c_str(), NULL, NULL, &err_msg);
		return occured_error_writes_message_to_log(ok, err_msg);
	}

	struct ImagePack
	{
		const int64 id;
		const Image source;
		const Image thumb;
		const std::string source_digest;
		const std::string thumb_digest;
	};

	const int64 get_unuse_id(sqlite3* db_connection)
	{
		sqlite3_stmt* statement;

		sqlite3_prepare_v2(db_connection, "select max(id) from image_t limit 1;", -1, &statement, nullptr);
		sqlite3_step(statement);
		int64 id = sqlite3_column_int64(statement, 0) + 1;
		sqlite3_finalize(statement);

		return id;
	}

	const ImagePack insert_image(sqlite3* db_connection, const Image& source, const int64 usually_id = -1, const bool use_transaction = true)
	{
		int64 id = usually_id;
		const double wh = source.width() > source.height() ? (double)source.width() : (double)source.height();
		const double scaling = 128.0 / wh;
		
		const Image thumb = source.scaled(scaling, s3d::Interpolation::Area);
		const std::string source_digest = Unicode::NarrowAscii(get_digest(source));
		const std::string thumb_digest = Unicode::NarrowAscii(get_digest(thumb));
		
		if (use_transaction)
		{
			sqlite3_exec(db_connection, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		}
		if (usually_id < 0)
		{
			id = get_unuse_id(db_connection);
		}
		ImagePack retval{ 
			id, std::move(source), std::move(thumb), 
			std::move(source_digest), std::move(thumb_digest) 
		};
		{
			sqlite3_stmt* statement;
			sqlite3_prepare_v2(
				db_connection,
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
			sqlite3_bind_text(statement, s_idigest, retval.source_digest.c_str(), (int)retval.source_digest.size(), nullptr);
			sqlite3_bind_blob(statement, s_idata, (void*)source.data(), (int)source.size_bytes(), nullptr);
			sqlite3_bind_int(statement, s_tw, retval.thumb.width());
			sqlite3_bind_int(statement, s_th, retval.thumb.height());
			sqlite3_bind_text(statement, s_tdigest, retval.thumb_digest.c_str(), (int)retval.thumb_digest.size(), nullptr);
			sqlite3_bind_blob(statement, s_tdata, (void*)retval.thumb.data(), (int)retval.thumb.size_bytes(), nullptr);
			sqlite3_step(statement);
			sqlite3_finalize(statement);
		}
		if (use_transaction)
		{
			sqlite3_exec(db_connection, "END TRANSACTION;", NULL, NULL, NULL);
		}

		return retval;
	}

	const Array<ImagePack> insert_images(sqlite3* db_connection, const Array<Image> imgs)
	{
		Array<ImagePack> retval;
		auto usually_id = get_unuse_id(db_connection);
		sqlite3_exec(db_connection, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		for (const auto& img : imgs)
		{
			retval.emplace_back(insert_image(db_connection, img, usually_id, false));
			++usually_id;
		}
		sqlite3_exec(db_connection, "END TRANSACTION;", NULL, NULL, NULL);

		return retval;
	}
}