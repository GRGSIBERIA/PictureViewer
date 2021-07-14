#pragma once
#include <Siv3D.hpp>

namespace classic
{
	class Widget
	{
	protected:
		const Color fore_color = Color(214, 207, 201);
		const Color frame_color = Palette::Black;
		const Color font_color = Palette::Black;

		const Font& font;
		const String text;
		const Size text_size;

	public:
		Widget(const Font& font, const String& text) 
			: font(font), text(text), text_size(font(text).region().size)
		{

		}
	};
}