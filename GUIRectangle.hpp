// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __GUI_RECTANGLE_H_INCLUDED__
#define __GUI_RECTANGLE_H_INCLUDED__


#include "IGUIElement.h"


class IGUIRectangle : public irr::gui::IGUIElement
{
public:
    //! constructor
    IGUIRectangle(irr::gui::IGUIEnvironment* environment, IGUIElement* parent, irr::core::rect<irr::s32> rectangle, bool showBorder=true);

    //! destructor
    virtual ~IGUIRectangle();

    //! draws the element and its children
    virtual void draw();

private:
    bool showBorder;
};

#endif

