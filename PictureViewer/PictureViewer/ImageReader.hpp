#pragma once
#include <Siv3D.hpp>
#include <sqlite3.h>

namespace image
{
	/// <summary>
	/// 画像を読み込む
	/// </summary>
	/// <param name="source">元になる画像のアドレス</param>
	/// <param name="width">幅</param>
	/// <param name="height">高さ</param>
	/// <returns>読み込み後のインスタンス</returns>
	Image read_image(void* source, const uint32& width, const uint32& height)
	{
		Image img(width, height);
		uint32 size = sizeof(Color) * width * height;
		
		void* dest = (void*)img.data();
		memcpy_s(dest, size, source, size);

		return img;
	}

	/// <summary>
	/// 画像の領域を確保して内容をコピーする
	/// </summary>
	/// <param name="source">元になる画像のインスタンス</param>
	/// <returns>コピーされた領域のアドレス</returns>
	void* reserve_image(const Image& source)
	{
		void* dest = sqlite3_malloc(source.size_bytes());
		memcpy_s(dest, source.size_bytes(), (void*)source.data(), source.size_bytes());

		return dest;
	}
}