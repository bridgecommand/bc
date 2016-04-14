// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

//#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "IGUIFont.h"
#include "IGUIFontBitmap.h"

#include "HeadingIndicator.h"

namespace irr
{
namespace gui
{


HeadingIndicator::HeadingIndicator(IGUIEnvironment* environment, IGUIElement* parent, core::rect<s32> rectangle) :
IGUIElement(EGUIET_ELEMENT ,environment,parent,0,rectangle), heading(0)
{
}

//! destructor
HeadingIndicator::~HeadingIndicator()
{
}

//! draws the element and its children
void HeadingIndicator::draw()
{

if (!IsVisible)
		return;

    IGUISkin* skin = Environment->getSkin();

    if (!skin)
        return;

    u32 skinAlpha = skin->getColor(gui::EGDC_3D_FACE).getAlpha();
    gui::IGUIFont* font = skin->getFont();

	// draws the background
	//skin->draw2DRectangle(this, skin->getColor(EGDC_SCROLLBAR), SliderRect, &AbsoluteClippingRect);
	//Environment->getVideoDriver()->draw2DRectangle(video::SColor(255,128,128,128),SliderRect,&AbsoluteClippingRect);
	Environment->getVideoDriver()->draw2DRectangleOutline(AbsoluteRect,video::SColor(skinAlpha,0,0,0)); //Todo: Think about clipping (find smaller of AbsoluteClippingRect and SliderRect?)

	//Find the centre point
	core::vector2d<s32> centrePoint = AbsoluteRect.getCenter();


	//Temp: Just draw the value
	font->draw(core::stringw(heading),core::rect<s32>(centrePoint,core::vector2d<s32>(100,100)),video::SColor(skinAlpha,0,0,0),false,false);



}

void HeadingIndicator::setHeading(f32 heading)
{
    this->heading = heading;
}

}
}


