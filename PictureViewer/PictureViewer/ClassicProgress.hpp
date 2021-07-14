#pragma once
#include <Siv3D.hpp>
#include "Widget.hpp"

namespace classic
{
	class Progress : public Widget
	{
		float val = 1.0f;
		float maximum = 1.0f;

	public:
		Progress(const Font& font)
			: Widget(font)
		{

		}

		virtual ~Progress() {}

		void update(float _val, float _maximum)
		{
			val = _val;
			maximum = _maximum;
		}

		const RectF draw(const Vec2& pos, const Vec2& pad, const float width) const
		{
			const String percent = U"{0:>3} %"_fmt((int)((val / maximum) * 100.0));
			
			auto reg = font(percent).region(pos);
			reg.w = width;
			reg.h += pad.y * 2.0;

			const float ratio = val / maximum;
			auto fill = reg;
			fill.w = ratio * width;
			fill.draw(fillC);

			auto avoid = reg;
			avoid.x = fill.x + fill.w;
			avoid.w = reg.w - fill.w;
			avoid.draw(avoidC);

			Line(reg.tl(), reg.tr()).draw(1, dentC);
			Line(reg.tl(), reg.bl()).draw(1, dentC);
			Line(reg.tr(), reg.br()).draw(1, specC);
			Line(reg.bl(), reg.br()).draw(1, specC);

			font(percent).draw(pos + pad, avoidC);

			return reg;
		}
	};
}