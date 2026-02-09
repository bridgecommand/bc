/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

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

#include "NumberToImage.hpp"
#include "irrlicht.h"
#include <vector>

//using namespace irr;

namespace NumberToImage
{

    const uint32_t PADDING_PX = 1;
    const irr::video::SColor BG_COLOUR = irr::video::SColor(0,0,0,0);

    irr::video::IImage* getImage(uint32_t number, irr::IrrlichtDevice* dev)
    {

        irr::core::stringc numberString = irr::core::stringc(number);
        const uint32_t length = numberString.size();
        //uint32_t imageWidth = length * CHAR_WIDTH;
        //uint32_t imageHeight = CHAR_HEIGHT;

        if (length > 0) {

			std::vector<irr::video::IImage*> numberImages;
			numberImages.resize(length);
            //video::IImage* numberImages[length];
            uint32_t overallWidth = 0;
            uint32_t maxHeight = 0;
            //video::IImage* numberImage = dev->getVideoDriver()->createImage(video::ECF_R8G8B8, irr::core::dimension2d<uint32_t>(imageWidth, imageHeight));

            for (uint32_t character = 0; character<length; character++) {
                //Load character image from file (charName.png)
                char thisChar = numberString.c_str()[character];

                irr::io::path imagePath = "media/Char";
                imagePath.append(thisChar);
                imagePath += ".png";

                numberImages[character] = dev->getVideoDriver()->createImageFromFile(imagePath);
                if (numberImages[character]) {
                    overallWidth+= numberImages[character]->getDimension().Width + PADDING_PX; //Padding at end
                    if (numberImages[character]->getDimension().Height > maxHeight) {
                        maxHeight = numberImages[character]->getDimension().Height;
                    }
                }


            }

            if (overallWidth>0) {
            	irr::video::IImage* numberImage = dev->getVideoDriver()->createImage(irr::video::ECF_A8R8G8B8, irr::core::dimension2d<uint32_t>(overallWidth, maxHeight));
                if (!numberImage) {
                    return 0;
                }

                numberImage->fill(BG_COLOUR); //Transparent
                //paste in the characters
                uint32_t nextXStart = 0;
                for (uint32_t character = 0; character<length; character++) {
                    if (numberImages[character]) {
                        irr::core::rect<int32_t> sourceRect = irr::core::rect<int32_t>(0,0,numberImages[character]->getDimension().Width,numberImages[character]->getDimension().Height);
                        numberImages[character]->copyToWithAlpha(numberImage,irr::core::vector2d<int32_t>(nextXStart,0),sourceRect,irr::video::SColor(255,255,255,255));
                        nextXStart += numberImages[character]->getDimension().Width + PADDING_PX;
                        numberImages[character]->drop();
                    }
                }
                return numberImage;
            } else {
                return 0;
            }

        } else {
            return 0;
        }

    }

}

