﻿
#include <Siv3D.hpp> // OpenSiv3D v0.4.3
#include "Database.hpp"
#include "Table.hpp"
#include "Config.hpp"
#include "Directory.hpp"

#include "ClassicButton.hpp"
#include "ClassicProgress.hpp"

void Main()
{
	const bool FORCE_DEBUG_MODE = true;

	// 背景を水色にする
	Scene::SetBackground(Color(214, 207, 201));

	setlocale(LC_ALL, "");

	// 大きさ 60 のフォントを用意
	const Font font(60);
	const Font fontS(12, FileSystem::SpecialFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc");
	conf::Config config = conf::initialize();

	classic::Button import_button(fontS, U"フォルダの読み込み");
	classic::Progress import_progress(fontS);
	
	db::connectDB();
	db::rollbackDB();

	if (FORCE_DEBUG_MODE)
		db::manually_drop_tables(db::connection);	// !!! テーブルを手動ですべて削除

	db::create_tables(db::connection);
	
	while (System::Update())
	{
		const auto import_reg = import_button.draw({ 8, 8 }, { 4, 4 });

		if (import_reg.leftClicked())
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

		const auto import_progress_reg = import_progress.draw(
			import_reg.tr() + Vec2{ 8, 0 }, { 4, 4 }, (float)Window::ClientWidth() - (import_reg.x + import_reg.w + 16));
	}

	db::closeDB();
}
