#pragma once
#include <Siv3D.hpp>

namespace classic
{
	class Widget
	{
	protected:
		const Color foreC = Color(214, 207, 201);
		const Color frameC = Palette::Black;
		const Color fontC = Palette::Black;
		const Color specC = Color(245, 243, 245);
		const Color shadowC = Color(23, 23, 23);
		const Color fillC = Color(1, 8, 133);
		const Color avoidC = Color(255, 255, 255);
		const Color dentC = Color(128, 128, 128);

		const Font& font;

	public:
		Widget(const Font& font) 
			: font(font)
		{

		}

		virtual ~Widget() {}
	};
}