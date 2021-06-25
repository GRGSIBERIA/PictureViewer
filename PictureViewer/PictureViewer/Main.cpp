
#include <Siv3D.hpp> // OpenSiv3D v0.4.3
#include "Database.hpp"
#include "Table.hpp"

void Main()
{
	// 背景を水色にする
	Scene::SetBackground(Palette::Gray);

	setlocale(LC_ALL, "");

	// 大きさ 60 のフォントを用意
	const Font font(60);

	db::connectDB();

	db::manuallyDropTables();	// !!! テーブルを手動ですべて削除

	db::initializeTables();

	while (System::Update())
	{
		
	}

	db::finalizeTables();

	db::closeDB();
}
