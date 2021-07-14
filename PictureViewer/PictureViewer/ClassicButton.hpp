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

			reg.draw(fore_color);					// �{�^���̘g����`��
			reg.drawFrame(1, frame_color);			// �t���[����`��
			font(text).draw(pos + pad, font_color); // ������`��

			return reg;
		}

		[[nodiscard]] const Size size() const
		{
			return text_size;
		}
	};
}