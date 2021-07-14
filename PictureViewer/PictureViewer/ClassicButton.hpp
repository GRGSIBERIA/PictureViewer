#pragma once
#include <Siv3D.hpp>

namespace classic
{
	class Button
	{
		const Color fore_color = Color(214, 207, 201);
		const Color frame_color = Palette::Black;
		const Color font_color = Palette::Black;

		const Font& font;
		const String text;
		const Size text_size;

	public:
		Button(const Font& font, const String& text)
			: font(font), text(text), text_size(font(text).region().size)
		{
			
		}

		const RectF draw(const Vec2& pos, const Vec2& pad) const
		{
			auto reg = font(text).region(pos);
			reg.size += 2.0 * pad;

			reg.draw(fore_color);					// ボタンの枠線を描画
			reg.drawFrame(1, frame_color);			// フレームを描画
			font(text).draw(pos + pad, font_color); // 文字を描画

			return reg;
		}

		[[nodiscard]] const Size size() const
		{
			return text_size;
		}
	};
}