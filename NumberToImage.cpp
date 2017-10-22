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
#include <vector>

using namespace irr;

namespace NumberToImage
{

    const u32 PADDING_PX = 1;
    const video::SColor BG_COLOUR = video::SColor(0,0,0,0);

    irr::video::IImage* getImage(irr::u32 number, irr::IrrlichtDevice* dev)
    {

        core::stringc numberString = core::stringc(number);
        const u32 length = numberString.size();
        //u32 imageWidth = length * CHAR_WIDTH;
        //u32 imageHeight = CHAR_HEIGHT;

        if (length > 0) {

			std::vector<video::IImage*> numberImages;
			numberImages.resize(length);
            //video::IImage* numberImages[length];
            u32 overallWidth = 0;
            u32 maxHeight = 0;
            //video::IImage* numberImage = dev->getVideoDriver()->createImage(video::ECF_R8G8B8, core::dimension2d<u32>(imageWidth, imageHeight));

            for (u32 character = 0; character<length; character++) {
                //Load character image from file (charName.png)
                char thisChar = numberString.c_str()[character];

                io::path imagePath = "media/Char";
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
                video::IImage* numberImage = dev->getVideoDriver()->createImage(video::ECF_A8R8G8B8, core::dimension2d<u32>(overallWidth, maxHeight));
                if (!numberImage) {
                    return 0;
                }

                numberImage->fill(BG_COLOUR); //Transparent
                //paste in the characters
                u32 nextXStart = 0;
                for (u32 character = 0; character<length; character++) {
                    if (numberImages[character]) {
                        core::rect<s32> sourceRect = core::rect<s32>(0,0,numberImages[character]->getDimension().Width,numberImages[character]->getDimension().Height);
                        numberImages[character]->copyToWithAlpha(numberImage,core::vector2d<s32>(nextXStart,0),sourceRect,video::SColor(255,255,255,255));
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

