#pragma once
#include "../Configuration/Macros.h"
#include "../SDLWrappers/Color.h"

#if _DRAW_UI_OUTLINES == 1
	static inline const nest::Color s_kUIContainerOutlineColor{ 255, 0, 0, 0 };
	static inline const nest::Color s_kUIWidgetOutlineColor{ 0, 255, 0, 0 };
	static constexpr int s_kPrintFontSize = 10;
#endif