// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __HEADING_INDICATOR_H_INCLUDED__
#define __HEADING_INDICATOR_H_INCLUDED__


#include "IGUIElement.h"

class HeadingIndicator : public irr::gui::IGUIElement
{
public:

    //! constructor
    HeadingIndicator(irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent, irr::core::rect<irr::s32> rectangle);

    //! destructor
    virtual ~HeadingIndicator();

    //! draws the element and its children
    virtual void draw();

    virtual void setHeading(irr::f32 heading);

private:
    irr::f32 heading;
};
#endif
