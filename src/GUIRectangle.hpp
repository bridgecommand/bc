// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __GUI_RECTANGLE_H_INCLUDED__
#define __GUI_RECTANGLE_H_INCLUDED__

#include "IGUIElement.h"

namespace irr
{
namespace gui
{

	class IGUIRectangle : public IGUIElement
	{
	public:

		//! constructor
		IGUIRectangle(IGUIEnvironment* environment, IGUIElement* parent, core::rect<s32> rectangle, bool showBorder=true);

		//! destructor
		virtual ~IGUIRectangle();

		//! draws the element and its children
		virtual void draw();

    private:
        bool showBorder;

    };
}
}

#endif

