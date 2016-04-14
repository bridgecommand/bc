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

    //Find integer angles within +- angleRange degrees of heading

    s32 angleRange = 20;

    s32 minAngle = ceil(heading - angleRange);
    s32 maxAngle = floor(heading + angleRange);

    f32 pxPerDegree = (f32)AbsoluteRect.getWidth()/(f32)(angleRange*2);

    //Draw tics
    core::vector2d<s32> startPoint;
	core::vector2d<s32> endPoint;

    for (int ticAng = minAngle; ticAng<= maxAngle; ticAng++) {
        startPoint.X = centrePoint.X + pxPerDegree*(ticAng-heading);
        endPoint.X = startPoint.X;
        endPoint.Y = AbsoluteRect.LowerRightCorner.Y;

        //Make tic length depend on whether it's a multiple of 10, 5 or other
        if (ticAng % 10 == 0) {
            startPoint.Y = 0.5*centrePoint.Y + 0.5*AbsoluteRect.UpperLeftCorner.Y;

            //Draw position
            s32 displayAng = ticAng;
            if (displayAng < 0) {displayAng+=360;}
            if (displayAng > 359) {displayAng-=360;}
            font->draw(core::stringw(displayAng),core::rect<s32>(startPoint.X - 100, AbsoluteRect.UpperLeftCorner.Y, startPoint.X + 100, AbsoluteRect.LowerRightCorner.Y),video::SColor(skinAlpha,0,0,0),true,false,&AbsoluteRect);

        } else if (ticAng % 5 == 0) {
            startPoint.Y = centrePoint.Y;
        } else {
            startPoint.Y = 0.5*centrePoint.Y + 0.5*AbsoluteRect.LowerRightCorner.Y;
        }

        Environment->getVideoDriver()->draw2DLine(startPoint,endPoint,video::SColor(skinAlpha,0,0,0));
    }

    //Draw a centre line
    startPoint.X = centrePoint.X;
    startPoint.Y = AbsoluteRect.UpperLeftCorner.Y;
    endPoint.X = centrePoint.X;
    endPoint.Y = AbsoluteRect.LowerRightCorner.Y;
    Environment->getVideoDriver()->draw2DLine(startPoint,endPoint,video::SColor(skinAlpha,0,0,0));

}

void HeadingIndicator::setHeading(f32 heading)
{
    this->heading = heading;
}

}
}


