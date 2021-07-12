#pragma once
#include <Siv3D.hpp>
#include <sqlite3.h>
#include <Windows.h>

#include "Query.hpp"

namespace db
{
	/// <summary>
	/// 画像からハッシュ値を取得する
	/// </summary>
	/// <param name="source">元の画像</param>
	/// <returns>SHA256のダイジェスト</returns>
	const String get_digest(const Image& source)
	{
		const DWORD CRYPT_SIZE = 32;
		DWORD size = CRYPT_SIZE;
		BYTE digest[CRYPT_SIZE];
		HCRYPTPROV prov;
		HCRYPTHASH hash;
		String retval = U"";

		CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
		CryptCreateHash(prov, CALG_SHA_256, 0, 0, &hash);
		CryptHashData(hash, (BYTE*)source.data(), (DWORD)source.size_bytes(), 0);

		CryptGetHashParam(hash, HP_HASHVAL, digest, &size, 0);

		std::string buf(digest, digest + CRYPT_SIZE);
		retval = Unicode::WidenAscii(buf);

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

	/// <summary>
	/// まだ使われていないimage_t.idを取得する
	/// </summary>
	/// <param name="db_connection"></param>
	/// <returns>max(image_t.id) + 1</returns>
	const int64 get_unuse_id(sqlite3* db_connection)
	{
		sqlite3_stmt* statement;

		sqlite3_prepare_v2(db_connection, "select max(id) from image_t limit 1;", -1, &statement, nullptr);
		sqlite3_step(statement);
		int64 id = sqlite3_column_int64(statement, 0) + 1;
		sqlite3_finalize(statement);

		return id;
	}

	const ImagePack insert_image(sqlite3* db_connection, const Image& source, const int64 usually_id = -1)
	{
		int64 id = usually_id;
		const double wh = source.width() > source.height() ? (double)source.width() : (double)source.height();
		const double scaling = 128.0 / wh;
		
		const Image thumb = source.scaled(scaling, s3d::Interpolation::Area);
		const std::string source_digest = Unicode::NarrowAscii(get_digest(source));
		const std::string thumb_digest = Unicode::NarrowAscii(get_digest(thumb));
		
		if (usually_id < 0)
		{
			id = get_unuse_id(db_connection);
		}

		ImagePack retval{ 
			id, std::move(source), std::move(thumb), 
			std::move(source_digest), std::move(thumb_digest) 
		};
		
		// メソッドの中でシングルトンにしている
		// 呼び出し元が限定されているので、こういう書き方ができる
		typedef std::shared_ptr<query::InsertImage> InsertImagePtr;
		static InsertImagePtr invoker = nullptr;
		if (invoker == nullptr)
			invoker = InsertImagePtr(new query::InsertImage(db_connection));
		invoker->insert(retval);

		return std::move(retval);
	}

	const Array<ImagePack> insert_images(sqlite3* db_connection, const Array<Image>& imgs)
	{
		Array<ImagePack> retval;
		auto usually_id = get_unuse_id(db_connection);
		sqlite3_exec(db_connection, "BEGIN TRANSACTION INSERT_IMAGE;", NULL, NULL, NULL);
		for (const auto& img : imgs)
		{
			retval.emplace_back(insert_image(db_connection, img, usually_id));
			++usually_id;
		}
		sqlite3_exec(db_connection, "COMMIT TRANSACTION INSERT_IMAGE;", NULL, NULL, NULL);

		return retval;
	}
}