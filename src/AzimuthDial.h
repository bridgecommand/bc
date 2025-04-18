// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __AZIMUTH_DIAL_H_INCLUDED__
#define __AZIMUTH_DIAL_H_INCLUDED__

#include "IGUIScrollBar.h"

namespace irr
{
namespace gui
{

	class AzimuthDial : public IGUIScrollBar
	{
	public:

		//! constructor
		AzimuthDial(core::position2d< s32 > centre, u32 radius, IGUIEnvironment* environment,
				IGUIElement* parent, s32 id, bool noclip=false);

		//! destructor
		virtual ~AzimuthDial();

		//! called if an event happened.
		virtual bool OnEvent(const SEvent& event);

		//! draws the element and its children
		virtual void draw();

		virtual void OnPostRender(u32 timeMs);


		//! gets the maximum value of the scrollbar.
		virtual s32 getMax() const;

		//! sets the maximum value of the scrollbar.
		virtual void setMax(s32 max);

		//! gets the minimum value of the scrollbar.
		virtual s32 getMin() const;

		//! sets the minimum value of the scrollbar.
		virtual void setMin(s32 min);

		//! gets the small step value
		virtual s32 getSmallStep() const;

		//! sets the small step value
		virtual void setSmallStep(s32 step);

		//! gets the large step value
		virtual s32 getLargeStep() const;

		//! sets the large step value
		virtual void setLargeStep(s32 step);

		//! gets the current position of the scrollbar
		virtual s32 getPos() const;

		//! sets the position of the scrollbar
		virtual void setPos(s32 pos);

		//! gets the current magnitude of the scrollbar
		virtual s32 getMag() const;

		//! sets the magnitude of the scrollbar
		virtual void setMag(s32 mag);

		//! updates the rectangle
		virtual void updateAbsolutePosition();

		//! Writes attributes of the element.
		//virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;

		//! Reads attributes of the element
		//virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

	private:

		//void refreshControls();
		s32 getPosFromMousePos(const core::position2di &p) const;
		s32 getMagFromMousePos(const core::position2di &p) const;

		//IGUIButton* UpButton;
		//IGUIButton* DownButton;

		core::rect<s32> SliderRect;

		core::position2d< s32 > centre;
		u32 radius;

		bool Dragging;
		//bool Horizontal;
		//bool DraggedBySlider;
		//bool TrayClick;
		s32 Pos;
		s32 Mag;
		f32 DrawRad;
		f32 DrawAngle;

		s32 DrawHeight;
		s32 Min;
		s32 Max;
		s32 SmallStep;
		s32 LargeStep;
		s32 DesiredPos;
		//u32 LastChange;
		video::SColor CurrentIconColor;

		f32 range () const { return (f32) ( Max - Min ); }
	};

} // end namespace gui
} // end namespace irr

#endif

