#pragma once
#include <Siv3D.hpp>

#include "Widget.hpp"

namespace classic
{
	class Button : public Widget
	{
	public:
		Button(const Font& font, const String& text)
			: Widget(font, text)
		{
			
		}

		const RectF draw(const Vec2& pos, const Vec2& pad) const
		{
			auto reg = font(text).region(pos);
			reg.size += 2.0 * pad;

			reg.draw(fore_color);					// ƒ{ƒ^ƒ“‚Ì˜gü‚ğ•`‰æ
			reg.drawFrame(1, frame_color);			// ƒtƒŒ[ƒ€‚ğ•`‰æ
			font(text).draw(pos + pad, font_color); // •¶š‚ğ•`‰æ

			return reg;
		}

		[[nodiscard]] const Size size() const
		{
			return text_size;
		}
	};
}