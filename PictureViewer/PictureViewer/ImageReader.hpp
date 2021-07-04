#pragma once
#include <Siv3D.hpp>
#include <sqlite3.h>
#include <Windows.h>

namespace image
{
	/// <summary>
	/// 画像を読み込む
	/// </summary>
	/// <param name="source">元になる画像のアドレス</param>
	/// <param name="width">幅</param>
	/// <param name="height">高さ</param>
	/// <returns>読み込み後のインスタンス</returns>
	Image read_image_from_bytes(void* source, const uint32& width, const uint32& height)
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
	void* reserve_address_from_image(const Image& source)
	{
		void* dest = sqlite3_malloc(source.size_bytes());
		memcpy_s(dest, source.size_bytes(), (void*)source.data(), source.size_bytes());

		return dest;
	}

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
}