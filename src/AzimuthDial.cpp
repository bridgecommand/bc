// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "IGUIFont.h"
#include "IGUIFontBitmap.h"

#include "AzimuthDial.h"

namespace irr
{
namespace gui
{


//! constructor
AzimuthDial::AzimuthDial(core::position2d< s32 > centre, u32 radius, IGUIEnvironment* environment,
				IGUIElement* parent, s32 id, bool noclip) :
				IGUIScrollBar(environment, parent, id, core::rect<s32>(centre.X - radius, centre.Y - radius, centre.X + radius,centre.Y + radius)),
				centre(centre), radius(radius),
				Dragging(false), Pos(0), Mag(0), DrawRad(0), DrawAngle(0), DrawHeight(0),
				Min(0), Max(360), SmallStep(10), LargeStep(50), DesiredPos(0)
{

	#ifdef _DEBUG
	setDebugName("AzimuthDial");
	#endif

//	refreshControls();

	setNotClipped(noclip);

	// this element can be tabbed to
	setTabStop(true);
	setTabOrder(-1);

	setPos(0);
	setMag(0);
}


//! destructor
AzimuthDial::~AzimuthDial()
{

}


//! called if an event happened.
bool AzimuthDial::OnEvent(const SEvent& event)
{
	if (isEnabled())
	{

		switch(event.EventType)
		{
		case EET_KEY_INPUT_EVENT:
			if (event.KeyInput.PressedDown)
			{
				const s32 oldPos = Pos;
				bool absorb = true;
				switch (event.KeyInput.Key)
				{
				case KEY_LEFT:
				case KEY_UP:
					setPos(Pos-SmallStep);
					break;
				case KEY_RIGHT:
				case KEY_DOWN:
					setPos(Pos+SmallStep);
					break;
				case KEY_HOME:
					setPos(Min);
					break;
				case KEY_PRIOR:
					setPos(Pos-LargeStep);
					break;
				case KEY_END:
					setPos(Max);
					break;
				case KEY_NEXT:
					setPos(Pos+LargeStep);
					break;
				default:
					absorb = false;
				}

				if (Pos != oldPos)
				{
					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
				}
				if (absorb)
					return true;
			}
			break;
		case EET_GUI_EVENT:
			if (event.GUIEvent.EventType == EGET_ELEMENT_FOCUS_LOST)
			{
				if (event.GUIEvent.Caller == this)
					Dragging = false;
			}
			break;
		case EET_MOUSE_INPUT_EVENT:
		{
			const core::position2di p(event.MouseInput.X, event.MouseInput.Y);
			bool isInside = isPointInside ( p );
			switch(event.MouseInput.Event)
			{
			case EMIE_MOUSE_WHEEL:
				if (Environment->hasFocus(this))
				{
					// thanks to a bug report by REAPER
					// thanks to tommi by tommi for another bugfix
					// everybody needs a little thanking. hallo niko!;-)
					setPos(	getPos() +
							( (event.MouseInput.Wheel < 0 ? -1 : 1) * SmallStep )
							);

					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
					return true;
				}
				break;
			case EMIE_LMOUSE_PRESSED_DOWN:
			case EMIE_RMOUSE_PRESSED_DOWN: //JAMES: Allow right click for scroll bar movement
			{
				if (isInside)
				{
					Dragging = true;
					//DraggedBySlider = SliderRect.isPointInside(p);
					//TrayClick = !DraggedBySlider;
					//DesiredPos = getPosFromMousePos(p);
					setPos(getPosFromMousePos(p));
					setMag(getMagFromMousePos(p));
					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
					Environment->setFocus ( this );
					return true;
				}
				break;
			}
			case EMIE_LMOUSE_LEFT_UP:
			case EMIE_RMOUSE_LEFT_UP: //JAMES: Allow right click for scroll bar movement
			case EMIE_MOUSE_MOVED:
			{
				if ( !event.MouseInput.isLeftPressed () && !event.MouseInput.isRightPressed ()  ) //JAMES: Allow right click for scroll bar movement
					Dragging = false;

				if ( !Dragging )
				{
					if ( event.MouseInput.Event == EMIE_MOUSE_MOVED )
						break;
					return isInside;
				}

				if ( event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP || event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP ) //JAMES: Allow right click for scroll bar movement
					Dragging = false;

				const s32 newPos = getPosFromMousePos(p);
				const s32 oldPos = Pos;
                setPos(newPos);
				const s32 newMag = getMagFromMousePos(p);
				const s32 oldMag = Mag;
                setMag(newMag);

				if (( (Pos != oldPos) || (Mag != oldMag) ) && Parent)
				{
					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
				}
				return isInside;
			} break;

			default:
				break;
			}
		} break;
		default:
			break;
		}
	}

	return IGUIElement::OnEvent(event);
}

void AzimuthDial::OnPostRender(u32 timeMs)
{

}

//! draws the element and its children
void AzimuthDial::draw()
{

	if (!IsVisible)
		return;


    IGUISkin* skin = Environment->getSkin();
    u32 skinAlpha = 255;
    if (skin) {
        skinAlpha = skin->getColor(gui::EGDC_3D_FACE).getAlpha();
    }

    s32 offsetX = AbsoluteRect.LowerRightCorner.X - RelativeRect.LowerRightCorner.X;
    s32 offsetY = AbsoluteRect.LowerRightCorner.Y - RelativeRect.LowerRightCorner.Y;

    core::vector2d<s32> absoluteCentre(centre.X + offsetX, centre.Y + offsetY);

	Environment->getVideoDriver()->draw2DPolygon(absoluteCentre,radius,video::SColor(skinAlpha,0,0,0),30);

	SliderRect = AbsoluteRect;

	if ( core::isnotzero ( range() ) )
	{
		//Draw from centre
		core::vector2d<s32> endPoint;
		endPoint.X = absoluteCentre.X + DrawRad*sin(DrawAngle);
		endPoint.Y = absoluteCentre.Y - DrawRad*cos(DrawAngle);

		if (DrawRad > 0) {
            // Normal
            Environment->getVideoDriver()->draw2DLine(absoluteCentre,endPoint,video::SColor(skinAlpha,0,0,0));
		} else {
		    // Draw in red if negative engine
		    Environment->getVideoDriver()->draw2DLine(absoluteCentre,endPoint,video::SColor(skinAlpha,255,0,0));
		}
	}

}


void AzimuthDial::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();
	// todo: properly resize
//	refreshControls();
	setPos ( Pos );
	setMag ( Mag );
}

//!
s32 AzimuthDial::getPosFromMousePos(const core::position2di &pos) const
{
	//Get the angle (range 0-360 degrees), and convert into output position
	s32 offsetX = AbsoluteRect.LowerRightCorner.X - RelativeRect.LowerRightCorner.X;
    s32 offsetY = AbsoluteRect.LowerRightCorner.Y - RelativeRect.LowerRightCorner.Y;

    s32 relX = pos.X - centre.X - offsetX;
    s32 relY = pos.Y - centre.Y - offsetY;

    f32 angle = atan2(relX,-1.0*relY)*core::RADTODEG;
    while (angle<0) {angle+=360;} //As atan2 gives -pi to +pi
    f32 proportion = angle/360.0;
    return (s32) (proportion * range()) + Min;
}

//!
s32 AzimuthDial::getMagFromMousePos(const core::position2di &pos) const
{
	//Get the magnitude
	s32 offsetX = AbsoluteRect.LowerRightCorner.X - RelativeRect.LowerRightCorner.X;
    s32 offsetY = AbsoluteRect.LowerRightCorner.Y - RelativeRect.LowerRightCorner.Y;

    s32 relX = pos.X - centre.X - offsetX;
    s32 relY = pos.Y - centre.Y - offsetY;

    f32 pixelRad = pow(pow(relX, 2.0) + pow(relY, 2.0), 0.5);
    f32 proportion = pixelRad/radius;
    return (s32) (proportion * 100); // Hardcoded as 0-100
}


//! sets the position of the scrollbar
void AzimuthDial::setPos(s32 pos)
{
	while (pos < Min) {
        pos += 360;
	}
	while (pos > Max) {
        pos -= 360;
	}

	Pos = pos;

	DrawAngle = (Pos-Min) * 360 / range() * core::DEGTORAD; //0-360 degrees for display
    DrawHeight = RelativeRect.getWidth();

}

//! gets the current magnitude of the scrollbar (0-100)
s32 AzimuthDial::getMag() const
{
	return Mag;
}

//! sets the magnitude of the scrollbar (-100-100)
void AzimuthDial::setMag(s32 mag)
{
	Mag = core::s32_clamp(mag, -100, 100);
	DrawRad = radius * ((f32) Mag / 100.0);
}


//! gets the small step value
s32 AzimuthDial::getSmallStep() const
{
	return SmallStep;
}


//! sets the small step value
void AzimuthDial::setSmallStep(s32 step)
{
	if (step > 0)
		SmallStep = step;
	else
		SmallStep = 10;
}


//! gets the small step value
s32 AzimuthDial::getLargeStep() const
{
	return LargeStep;
}


//! sets the small step value
void AzimuthDial::setLargeStep(s32 step)
{
	if (step > 0)
		LargeStep = step;
	else
		LargeStep = 50;
}


//! gets the maximum value of the scrollbar.
s32 AzimuthDial::getMax() const
{
	return Max;
}


//! sets the maximum value of the scrollbar.
void AzimuthDial::setMax(s32 max)
{
	Max = max;
	if ( Min > Max )
		Min = Max;

//	bool enable = core::isnotzero ( range() );
	setPos(Pos);
}

//! gets the minimum value of the scrollbar.
s32 AzimuthDial::getMin() const
{
	return Min;
}


//! sets the minimum value of the scrollbar.
void AzimuthDial::setMin(s32 min)
{
	Min = min;
	if ( Max < Min )
		Max = Min;


//	bool enable = core::isnotzero ( range() );
	setPos(Pos);
}


//! gets the current position of the scrollbar
s32 AzimuthDial::getPos() const
{
	return Pos;
}

/*
//! refreshes the position and text on child buttons
void AzimuthDial::refreshControls()
{
	CurrentIconColor = video::SColor(255,255,255,255);

	IGUISkin* skin = Environment->getSkin();

	if (skin)
	{
		CurrentIconColor = skin->getColor(isEnabled() ? EGDC_WINDOW_SYMBOL : EGDC_GRAY_WINDOW_SYMBOL);
	}

	if (Horizontal)
	{
		s32 h = RelativeRect.getHeight();
	}
	else
	{
		s32 w = RelativeRect.getWidth();
	}
}
*/

/*
//! Writes attributes of the element.
void AzimuthDial::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUIScrollBar::serializeAttributes(out,options);

	out->addBool("Horizontal",	Horizontal);
	out->addInt ("Value",		Pos);
	out->addInt ("Min",			Min);
	out->addInt ("Max",			Max);
	out->addInt ("SmallStep",	SmallStep);
	out->addInt ("LargeStep",	LargeStep);
	// CurrentIconColor - not serialized as continuiously updated
}


//! Reads attributes of the element
void AzimuthDial::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUIScrollBar::deserializeAttributes(in,options);

	Horizontal = in->getAttributeAsBool("Horizontal");
	setMin(in->getAttributeAsInt("Min"));
	setMax(in->getAttributeAsInt("Max"));
	setPos(in->getAttributeAsInt("Value"));
	setSmallStep(in->getAttributeAsInt("SmallStep"));
	setLargeStep(in->getAttributeAsInt("LargeStep"));
	// CurrentIconColor - not serialized as continuiously updated

	refreshControls();
}
*/

} // end namespace gui
} // end namespace irr

