#pragma once
#include <Siv3D.hpp>

#include "Widget.hpp"

namespace classic
{
	class Button : public Widget
	{
		const String text;

	public:
		Button(const Font& font, const String& text)
			: Widget(font), text(text)
		{
			
		}

		virtual ~Button() {}

		const RectF draw(const Vec2& pos, const Vec2& pad) const
		{
			auto reg = region(pos, pad);

			Line(reg.tl(), reg.tr()).draw(1, specC);
			Line(reg.tl(), reg.bl()).draw(1, specC);
			Line(reg.tr(), reg.br()).draw(1, shadowC);
			Line(reg.bl(), reg.br()).draw(1, shadowC);

			reg.draw(foreC);					// ƒ{ƒ^ƒ“‚Ì˜gü‚ğ•`‰æ

			font(text).draw(pos + pad, fontC); // •¶š‚ğ•`‰æ

			return reg;
		}

		const RectF region(const Vec2& pos, const Vec2& pad = Vec2{ 0, 0 }) const
		{
			auto reg = font(text).region(pos);
			reg.size += 2.0 * pad;
			return reg;
		}
	};
}