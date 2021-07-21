#pragma once
#include <Siv3D.hpp>
#include <filesystem>
#include <algorithm>
#include <future>
#include <thread>

#include "ClassicProgress.hpp"

namespace dir
{
	namespace fs = std::filesystem;

	Array<String> get_filenames(const String& target_dir)
	{
		Array<String> files;

		const auto pathes = s3d::FileSystem::DirectoryContents(target_dir);

		for (const auto& path : pathes)
		{
			const auto ext = FileSystem::Extension(path).lowercased();
			
			if (ext == U"png" || ext == U"jpg" || ext == U"jpeg")
			{
				files.emplace_back(path);
			}
		}
		return files;
	}

	const Array<Image> get_images(const Array<String>& files, classic::Progress& progress)
	{
		Array<Image> images;

		int i = 0;
		for (const auto& file : files)
		{
			auto img = s3d::Image(file);
			images.push_back(img);
			++i;
			progress.update((float)i, (float)file.size() * 2.0f);
		}

		return images;
	}
}