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

#ifndef __TERRAIN_HPP_INCLUDED__
#define __TERRAIN_HPP_INCLUDED__

#include <cstdint>
#include <string>
#include <vector>

// Forward declarations for Irrlicht types still needed internally
namespace irr {
    class IrrlichtDevice;
    namespace scene {
        class ISceneManager;
        class ITerrainSceneNode;
        class ISceneNode;
    }
    namespace io {
        class IReadFile;
    }
}

class Terrain
{
    public:
        Terrain();
        virtual ~Terrain();
        void load(const std::string& worldPath, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* device, uint32_t terrainResolutionLimit);
        float longToX(float longitude) const;
        float latToZ(float latitude) const;
        float xToLong(float x) const;
        float zToLat(float z) const;
        float getHeight(float x, float z) const;
        void moveNode(float deltaX, float deltaY, float deltaZ);
        void addRadarReflectingTerrain(std::vector<std::vector<float>> heightVector, float positionX, float positionZ, float widthX, float widthZ);
        irr::scene::ISceneNode* getSceneNode(int number);

    private:

        std::vector<std::vector<float>> heightMapImageToVector(irr::io::IReadFile* heightMapFile, bool usesRGBEncoding, bool normaliseSize, irr::scene::ISceneManager* smgr);
        std::vector<std::vector<float>> heightMapBinaryToVector(irr::io::IReadFile* heightMapFile, uint32_t binaryWidth, uint32_t binaryHeight, bool floatingPoint);

        std::vector<std::vector<float>> transposeHeightMapVector(std::vector<std::vector<float>> inVector);
        std::vector<std::vector<float>> limitSize(std::vector<std::vector<float>> inVector, uint32_t maxSize);

        irr::IrrlichtDevice* dev;

        std::vector<irr::scene::ITerrainSceneNode*> terrains;
        float primeTerrainLong;
        float primeTerrainXWidth;
        float primeTerrainLongExtent;
        float primeTerrainLat;
        float primeTerrainZWidth;
        float primeTerrainLatExtent;
};

#endif
