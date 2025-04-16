// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

//#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IVideoDriver.h"

#include "GUIRectangle.hpp"

namespace irr
{
namespace gui
{


IGUIRectangle::IGUIRectangle(IGUIEnvironment* environment, IGUIElement* parent, irr::core::rect<irr::s32> rectangle, bool showBorder) :
IGUIElement(EGUIET_ELEMENT,environment,parent,0,rectangle)
{
    this->showBorder=showBorder;
}

//! destructor
IGUIRectangle::~IGUIRectangle()
{
}

//! draws the element and its children
void IGUIRectangle::draw()
{

if (!IsVisible)
		return;

    IGUISkin* skin = Environment->getSkin();

    if (!skin)
        return;

    irr::u32 skinAlpha = skin->getColor(irr::gui::EGDC_3D_FACE).getAlpha();

	// draws the background
	//skin->draw2DRectangle(this, skin->getColor(EGDC_SCROLLBAR), SliderRect, &AbsoluteClippingRect);
	//Environment->getVideoDriver()->draw2DRectangle(video::SColor(255,128,128,128),SliderRect,&AbsoluteClippingRect);
	if (showBorder)
        Environment->getVideoDriver()->draw2DRectangleOutline(AbsoluteRect,video::SColor(skinAlpha,0,0,0)); //Todo: Think about clipping (find smaller of AbsoluteClippingRect and SliderRect?)

	//Draw children
	IGUIElement::draw();

}

}
}



