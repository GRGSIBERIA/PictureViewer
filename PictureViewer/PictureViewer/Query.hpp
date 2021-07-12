#pragma once
#include <sqlite3.h>
#include <Siv3D.hpp>

#include "Database.hpp"

namespace query
{
	class Query
	{
	protected:
		sqlite3_stmt* statement = nullptr;
		sqlite3* db;

	public:
		Query(sqlite3* _db) : db(_db) {}

		virtual ~Query()
		{
			sqlite3_finalize(statement);
		}
	};

	class InsertImage : public Query
	{
		int s_id;
		int s_iw;
		int s_ih;
		int s_idigest;
		int s_idata;
		int s_tw;
		int s_th;
		int s_tdigest;
		int s_tdata;

	public:
		InsertImage(sqlite3* db)
			: Query(db)
		{
			sqlite3_prepare_v2(
				db,
				"insert"
				"  into image_t(id, width, height, digest, data) values(:id, :iw, :ih, :idigest, :idata);"
				"insert"
				"  into thumb_t(imageid, width, height, digest, data) values (:id, :tw, :th, :tdigest, :tdata);",
				-1, &statement, nullptr);

			s_id = sqlite3_bind_parameter_index(statement, ":id");
			s_iw = sqlite3_bind_parameter_index(statement, ":iw");
			s_ih = sqlite3_bind_parameter_index(statement, ":ih");
			s_idigest = sqlite3_bind_parameter_index(statement, ":idigest");
			s_idata = sqlite3_bind_parameter_index(statement, ":idata");
			s_tw = sqlite3_bind_parameter_index(statement, ":tw");
			s_th = sqlite3_bind_parameter_index(statement, ":th");
			s_tdigest = sqlite3_bind_parameter_index(statement, ":tdigest");
			s_tdata = sqlite3_bind_parameter_index(statement, ":tdata");
		}

		void insert(const db::ImagePack& retval)
		{
			sqlite3_bind_int64(statement, s_id, retval.id);
			sqlite3_bind_int(statement, s_iw, retval.source.width());
			sqlite3_bind_int(statement, s_ih, retval.source.height());
			sqlite3_bind_text(statement, s_idigest, retval.source_digest.c_str(), (int)retval.source_digest.size(), nullptr);
			sqlite3_bind_blob(statement, s_idata, (void*)retval.source.data(), (int)retval.source.size_bytes(), nullptr);
			sqlite3_bind_int(statement, s_tw, retval.thumb.width());
			sqlite3_bind_int(statement, s_th, retval.thumb.height());
			sqlite3_bind_text(statement, s_tdigest, retval.thumb_digest.c_str(), (int)retval.thumb_digest.size(), nullptr);
			sqlite3_bind_blob(statement, s_tdata, (void*)retval.thumb.data(), (int)retval.thumb.size_bytes(), nullptr);

			auto ok = sqlite3_step(statement);
			if (ok != SQLITE_DONE)
			{
				const auto* msg = sqlite3_errmsg(db);
				Logger << Unicode::WidenAscii(msg);
			}

			sqlite3_reset(statement);
		}

		virtual ~InsertImage() {}
	};
}