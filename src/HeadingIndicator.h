// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __HEADING_INDICATOR_H_INCLUDED__
#define __HEADING_INDICATOR_H_INCLUDED__

#include "IGUIElement.h"

namespace irr
{
namespace gui
{

	class HeadingIndicator : public IGUIElement
	{
	public:

		//! constructor
		HeadingIndicator(IGUIEnvironment* environment, IGUIElement* parent, core::rect<s32> rectangle);

		//! destructor
		virtual ~HeadingIndicator();

		//! draws the element and its children
		virtual void draw();

		virtual void setHeading(f32 heading);

    private:

        f32 heading;

    };
}
}

#endif
