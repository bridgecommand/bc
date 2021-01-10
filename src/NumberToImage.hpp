/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2016 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef __NUMBERTOIMAGE_HPP_INCLUDED__
#define __NUMBERTOIMAGE_HPP_INCLUDED__

#include "irrlicht.h"

namespace NumberToImage
{

    irr::video::IImage* getImage(irr::u32 number, irr::IrrlichtDevice* dev);

}

#endif // __NUMBERTOIMAGE_HPP_INCLUDED__
