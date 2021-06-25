
#include <Siv3D.hpp> // OpenSiv3D v0.4.3
#include "Database.hpp"

void Main()
{
	// 背景を水色にする
	Scene::SetBackground(Palette::Gray);

	setlocale(LC_ALL, "");

	// 大きさ 60 のフォントを用意
	const Font font(60);

	sqlite3_open(db::path.toUTF8().c_str(), &db::connection);

	while (System::Update())
	{
		
	}

	sqlite3_close(db::connection);
}
