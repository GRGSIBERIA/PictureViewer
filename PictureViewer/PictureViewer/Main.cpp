
#include <Siv3D.hpp> // OpenSiv3D v0.4.3
#include "Database.hpp"
#include "Table.hpp"
#include "Config.hpp"

void Main()
{
	// 背景を水色にする
	Scene::SetBackground(Palette::Gray);

	setlocale(LC_ALL, "");

	// 大きさ 60 のフォントを用意
	const Font font(60);
	conf::Config config = conf::initialize();
	
	db::connectDB();

	db::manually_drop_tables(db::connection);	// !!! テーブルを手動ですべて削除
	db::create_tables(db::connection);
	
	while (System::Update())
	{
		if (SimpleGUI::Button(U"Import", { 0, 0 }))
		{
			s3d::Dialog::SelectFolder();
		}
	}

	db::closeDB();
}
