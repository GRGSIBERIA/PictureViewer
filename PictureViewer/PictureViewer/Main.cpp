
#include <Siv3D.hpp> // OpenSiv3D v0.4.3
#include "Database.hpp"
#include "Table.hpp"
#include "Config.hpp"
#include "Directory.hpp"



void Main()
{
	const bool FORCE_DEBUG_MODE = true;

	// 背景を水色にする
	Scene::SetBackground(Palette::Gray);

	setlocale(LC_ALL, "");

	// 大きさ 60 のフォントを用意
	const Font font(60);
	conf::Config config = conf::initialize();
	
	db::connectDB();
	db::rollbackDB();

	if (FORCE_DEBUG_MODE)
		db::manually_drop_tables(db::connection);	// !!! テーブルを手動ですべて削除

	db::create_tables(db::connection);
	
	while (System::Update())
	{
		if (SimpleGUI::Button(U"Import", { 0, 0 }))
		{
			const auto path = s3d::Dialog::SelectFolder();
			
			path.then([](const auto& p) 
				{
					std::thread task([p]() {
						const auto files = dir::get_filenames(p);
						const auto images = dir::get_images(files);
						db::insert_images(db::connection, images);
						});
					task.detach();
				});
		}
	}

	db::closeDB();
}
