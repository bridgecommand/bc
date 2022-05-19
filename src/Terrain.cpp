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

//NOTE: This uses a modified version of Irrlicht for terrain loading, which loads the terrain natively rotated
//180 degrees compared to standard Irrlicht 1.8.1.

#include "Terrain.hpp"

#include "IniFile.hpp"
#include "Constants.hpp"
#include "Utilities.hpp"

#include "BCTerrainSceneNode.h"

#include <iostream>
#include <algorithm>

//using namespace irr;

Terrain::Terrain()
{

}

Terrain::~Terrain()
{
    //dtor
    for (unsigned int i=0; i<terrains.size(); i++) {
        terrains.at(i)->drop();
    }
}

void Terrain::load(const std::string& worldPath, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* device, irr::u32 terrainResolutionLimit)
{

    dev = device;
    
    irr::video::IVideoDriver* driver = smgr->getVideoDriver();

    //Get full path to the main Terrain.ini file
    std::string worldTerrainFile = worldPath;
    worldTerrainFile.append("/terrain.ini");

    irr::u32 numberOfTerrains=0;
    bool usingHdrFileOnly = false;

    //If terrain.ini doesn't exist, look for *.bin and *.hdr
    if (!Utilities::pathExists(worldTerrainFile)) {
        irr::io::IFileSystem* fileSystem = device->getFileSystem();
        
        //store current dir
        irr::io::path cwd = fileSystem->getWorkingDirectory();

        //change to scenario dir
        if (fileSystem->changeWorkingDirectoryTo(worldPath.c_str())) {
            irr::io::IFileList* fileList = fileSystem->createFileList();
            if (fileList!=0) {
                //List here
                for (irr::u32 i=0;i<fileList->getFileCount();i++) {
                    if (fileList->isDirectory(i)==false) {
                        const irr::io::path& fileName = fileList->getFileName(i);
                        if (irr::core::hasFileExtension(fileName,"hdr")) {
                            //Found a .hdr file for us to use
                            
                            //TODO: Check if equivalent .bin exists here
                            
                            worldTerrainFile = worldPath;
                            worldTerrainFile.append("/");
                            worldTerrainFile.append(fileName.c_str());
                            numberOfTerrains=1;
                            usingHdrFileOnly = true;
                            std::cout << "Found hdr file " << worldTerrainFile << std::endl; 
                        }
                    }
                }
            }
        }

        //Change dir back
        fileSystem->changeWorkingDirectoryTo(cwd);
    } else {
        numberOfTerrains = IniFile::iniFileTou32(worldTerrainFile, "Number");
    }

    if (numberOfTerrains <= 0) {
        std::cerr << "Could not load terrain. No terrain defined in settings file." << std::endl;
        exit(EXIT_FAILURE);
    }

    for (unsigned int i = 1; i<=numberOfTerrains; i++) {

        irr::f32 terrainLong;
        irr::f32 terrainLat;
        irr::f32 terrainLongExtent;
        irr::f32 terrainLatExtent;
        irr::f32 terrainMaxHeight;
        irr::f32 seaMaxDepth;
        bool usesRGBEncoding;
        //Full paths
        std::string heightMapPath;
        std::string textureMapPath;
        
        if (usingHdrFileOnly) {
            //load from 3dem header format, worldTerrainFile is the .hdr file in this case
            terrainLong = IniFile::iniFileTof32(worldTerrainFile, "left_map_x");
            terrainLat = IniFile::iniFileTof32(worldTerrainFile, "lower_map_y");
            terrainLongExtent = IniFile::iniFileTof32(worldTerrainFile, "right_map_x")-terrainLong;
            terrainLatExtent = IniFile::iniFileTof32(worldTerrainFile, "upper_map_y")-terrainLat;

            heightMapPath = worldTerrainFile; //The name of the .hdr file
            
            //assume map is the same path, with .hdr replaced with .bmp
            //Use this as a texture (is this sensible?!)
            std::string textureMapName = std::string(device->getFileSystem()->getFileBasename(worldTerrainFile.c_str(),false).append(".bmp").c_str());
            textureMapPath = worldPath;
            textureMapPath.append("/");
            textureMapPath.append(textureMapName);

            //Dummy contents, won't be used in this case
            terrainMaxHeight=0;
            seaMaxDepth=0;
            usesRGBEncoding=false;

        } else {
            //Load from normal terrain.ini format
            terrainLong = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLong",i));
            terrainLat = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLat",i));
            terrainLongExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLongExtent",i));
            terrainLatExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLatExtent",i));

            terrainMaxHeight=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainMaxHeight",i));
            seaMaxDepth=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("SeaMaxDepth",i));
            //irr::f32 terrainHeightMapSize=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapSize",i));

            std::string heightMapName = IniFile::iniFileToString(worldTerrainFile, IniFile::enumerate1("HeightMap",i));
            std::string textureMapName = IniFile::iniFileToString(worldTerrainFile, IniFile::enumerate1("Texture",i));

            usesRGBEncoding = IniFile::iniFileTou32(worldTerrainFile, IniFile::enumerate1("UsesRGB",i)) > 0;

            //Full paths
            heightMapPath = worldPath;
            heightMapPath.append("/");
            heightMapPath.append(heightMapName);
            
            textureMapPath = worldPath;
            textureMapPath.append("/");
            textureMapPath.append(textureMapName);
        }

        //calculations just needed for terrain loading
        //irr::f32 scaleX = terrainXWidth / (terrainHeightMapSize);
        irr::f32 scaleY = (terrainMaxHeight + seaMaxDepth)/ (255.0);
        //irr::f32 scaleZ = terrainZWidth / (terrainHeightMapSize);
        irr::f32 terrainY = -1*seaMaxDepth;

        //Add an empty terrain
        //irr::scene::ITerrainSceneNode* terrain = smgr->addTerrainSceneNode("",0,-1,irr::core::vector3df(0.f, terrainY, 0.f),irr::core::vector3df(0.f, 0.f, 0.f),irr::core::vector3df(1,1,1),irr::video::SColor(255,255,255,255),5,irr::scene::ETPS_9,0,true);

        irr::scene::BCTerrainSceneNode* terrain = new irr::scene::BCTerrainSceneNode(
            device,
            smgr->getRootSceneNode(), 
            smgr,
			smgr->getFileSystem(), -1, 5, irr::scene::ETPS_33
        );

        //Load the map
        irr::io::IReadFile* heightMapFile = smgr->getFileSystem()->createAndOpenFile(heightMapPath.c_str());
        //Check the height map file has loaded and the terrain exists
        if (terrain==0 || heightMapFile == 0) {
            //Could not load terrain
            std::cerr << "Could not load terrain. Height map file not loaded. " << heightMapPath << std::endl;
            std::cerr << "Terrain: " << terrain << " heightMapFile: " << heightMapFile << std::endl;
            exit(EXIT_FAILURE);
        }

        //Load the terrain and check success
        bool loaded = false;
        irr::f32 terrainXLoadScaling = 1;
        irr::f32 terrainZLoadScaling = 1;
        
        //Check extension
        std::string extension = "";
        if (heightMapPath.length() > 3) {
            extension = heightMapPath.substr(heightMapPath.length() - 4,4);
            Utilities::to_lower(extension);
        }
        
        if (extension.compare(".f32") == 0 ) {
            
            //Binary file

            
            irr::u32 binaryRows = IniFile::iniFileTou32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapRows",i));
            irr::u32 binaryCols = IniFile::iniFileTou32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapColumns",i));
            bool flipRowCol = false;

            //Fall back to TerrainHeightMapSize if not set - legacy file loading, also indicating swapped rows and columns
            if (binaryRows == 0 || binaryCols == 0) {
                binaryRows = IniFile::iniFileTou32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapSize",i));    
                binaryCols = IniFile::iniFileTou32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapSize",i));    
                flipRowCol = true;
            }

            //Load from binary file into a vector, 
            std::vector<std::vector<irr::f32>> heightMapVector = heightMapBinaryToVector(heightMapFile,binaryRows,binaryCols,true);
            
            //limit size if needed
            if (terrainResolutionLimit>0) {
                heightMapVector = limitSize(heightMapVector,terrainResolutionLimit);
            }
            
            //Need to flip row and columns for legacy files
            if (flipRowCol) {
                heightMapVector = transposeHeightMapVector(heightMapVector);
            }
            //Then use this vector to load terrain
            loaded = terrain->loadHeightMapVector(heightMapVector, terrainXLoadScaling, terrainZLoadScaling, irr::video::SColor(255, 255, 255, 255), 0);

        }  else if (extension.compare(".hdr") == 0 ) {
            //3Dem header for binary file
            irr::u32 binaryRows = IniFile::iniFileTou32(heightMapPath, "number_of_rows");
            irr::u32 binaryCols = IniFile::iniFileTou32(heightMapPath, "number_of_columns");

            terrainLong = IniFile::iniFileTof32(heightMapPath, "left_map_x");
            terrainLat = IniFile::iniFileTof32(heightMapPath, "lower_map_y");
            terrainLongExtent = IniFile::iniFileTof32(heightMapPath, "right_map_x")-terrainLong;
            terrainLatExtent = IniFile::iniFileTof32(heightMapPath, "upper_map_y")-terrainLat;

            bool floatingPoint = IniFile::iniFileToString(heightMapPath, "data_format").compare("float32")==0;

            //TODO: Check a floating point example

            //Assume (TODO, could check and allow different?)
            //data_format            = int16 or float32
            //map_projection         = Lat/Lon
            //elev_m_unit            = meters

            //Load from binary file into map, so close the .hdr, and open the .bin version
            heightMapFile->drop();
            heightMapPath.erase(heightMapPath.end()-3,heightMapPath.end());
            heightMapPath.append("bin");
            irr::io::IReadFile* heightMapFile = smgr->getFileSystem()->createAndOpenFile(heightMapPath.c_str());
            if (heightMapFile) {
                try {
                    //Load from binary file into a vector, and then use this vector to load terrain
                    std::vector<std::vector<irr::f32>> heightMapVector = heightMapBinaryToVector(heightMapFile,binaryRows,binaryCols,floatingPoint);
                    //limit size if needed
                    if (terrainResolutionLimit>0) {
                        heightMapVector = limitSize(heightMapVector,terrainResolutionLimit);
                    }
                    loaded = terrain->loadHeightMapVector(heightMapVector, terrainXLoadScaling, terrainZLoadScaling, irr::video::SColor(255, 255, 255, 255), 0);
                } catch (...) {
                    std::cerr << "Exception in loading terrain from binary with hdr." << std::endl;
                    loaded = false;    
                }
            }

        } else {
            //Normal image file
            std::vector<std::vector<irr::f32>> heightMapVector = heightMapImageToVector(heightMapFile,usesRGBEncoding,false,smgr);
            
            //limit size if needed
            if (terrainResolutionLimit>0) {
                heightMapVector = limitSize(heightMapVector,terrainResolutionLimit);
            }

            loaded = terrain->loadHeightMapVector(heightMapVector, terrainXLoadScaling, terrainZLoadScaling, irr::video::SColor(255, 255, 255, 255), 0);
        }

        if (!loaded) {
            //Could not load terrain
            std::cerr << "Could not load terrain at loadHeightMap stage." << std::endl;
            exit(EXIT_FAILURE);
        }

        //Terrain dimensions in metres
        irr::f32 terrainXWidth = terrainLongExtent * 2.0 * PI * EARTH_RAD_M * cos( irr::core::degToRad(terrainLat + terrainLatExtent/2.0)) / 360.0;
        irr::f32 terrainZWidth = terrainLatExtent  * 2.0 * PI * EARTH_RAD_M / 360;

        irr::f32 scaleX = terrainXLoadScaling*terrainXWidth/(terrain->getBoundingBox().MaxEdge.X - terrain->getBoundingBox().MinEdge.X);
        irr::f32 scaleZ = terrainZLoadScaling*terrainZWidth/(terrain->getBoundingBox().MaxEdge.Z - terrain->getBoundingBox().MinEdge.Z);
            
        if (extension.compare(".f32") == 0 || extension.compare(".hdr") == 0 || usesRGBEncoding) {
            //Set scales etc to be 1.0, so heights are used directly
            terrain->setScale(irr::core::vector3df(scaleX,1.0f,scaleZ));
            terrain->setPosition(irr::core::vector3df(0.f, 0.f, 0.f));
        } else {
            //Normal heightmap, so use scale from terrainMaxHeight etc
            terrain->setScale(irr::core::vector3df(scaleX,scaleY,scaleZ));
            terrain->setPosition(irr::core::vector3df(0.f, terrainY, 0.f));
        }

        heightMapFile->drop();
        //terrains are dropped in destructor.

        terrain->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
        terrain->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting
        //Todo: Anti-aliasing flag?
        terrain->setMaterialTexture(0, driver->getTexture(textureMapPath.c_str()));

        if (i==1) {
            //Private member variables used in further calculations
            primeTerrainLong = terrainLong;
            primeTerrainXWidth = terrainXWidth;
            primeTerrainLongExtent = terrainLongExtent;
            primeTerrainLat = terrainLat;
            primeTerrainZWidth = terrainZWidth;
            primeTerrainLatExtent = terrainLatExtent;


        } else {
            //Non-primary terrains need to be moved to account for their position (primary terrain starts at 0,0)

            irr::core::vector3df currentPos = terrain->getPosition();
            irr::f32 deltaX = (terrainLong - primeTerrainLong) * primeTerrainXWidth / primeTerrainLongExtent;
            irr::f32 deltaZ = (terrainLat - primeTerrainLat) * primeTerrainZWidth / primeTerrainLatExtent;
            irr::f32 newPosX = currentPos.X + deltaX;
            irr::f32 newPosY = currentPos.Y;
            irr::f32 newPosZ = currentPos.Z + deltaZ;
            terrain->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
        }

        terrains.push_back(terrain);

    }


}

std::vector<std::vector<irr::f32>> Terrain::heightMapImageToVector(irr::io::IReadFile* heightMapFile, bool usesRGBEncoding, bool normaliseSize, irr::scene::ISceneManager* smgr)
{
    irr::video::IImage* heightMap = smgr->getVideoDriver()->createImageFromFile(heightMapFile);
    std::vector<std::vector<irr::f32>> heightMapVector;

    if (heightMap==0) {
        //Return empty vector if we can't load the image
        return heightMapVector;
    }

    irr::u32 imageWidth = heightMap->getDimension().Width;
    irr::u32 imageHeight = heightMap->getDimension().Height;

    irr::s32 scaledWidth; 
    irr::s32 scaledHeight;
    if (normaliseSize) {
        //Find nearest 2^n+1 square size, upscaling if needed. 
        //Subtract 1 and find next power of 2, and add one (we need a size that is (2^n)+1)
        scaledWidth= (irr::s32)imageWidth-1;
        scaledHeight = (irr::s32)imageHeight-1;
        
        scaledWidth = pow(2.0,ceil(log2(scaledWidth))) + 1;
        scaledHeight = pow(2.0,ceil(log2(scaledHeight))) + 1;
        
        //find largest, returned vector will be square
        scaledWidth = std::max(scaledWidth,scaledHeight);
        scaledHeight = scaledWidth;
    } else {
        scaledWidth= (irr::s32)imageWidth;
        scaledHeight = (irr::s32)imageHeight;
    }
    
    irr::f32 heightValue;

    for (unsigned int k=0; k<scaledHeight; k++) {
        std::vector<irr::f32> heightMapLine;
        for (unsigned int j=0; j<scaledWidth; j++) {
            
            //Pick the pixel to use (very simple scaling, to be replaced with bilinear interpolation)
            irr::f32 pixelX_float = (irr::f32)j * (irr::f32)imageWidth/(irr::f32)scaledWidth;
            irr::f32 pixelY_float = (irr::f32)(scaledHeight - k - 1) * (irr::f32)imageHeight/(irr::f32)scaledHeight;
            irr::u32 pixelX_int = round(pixelX_float);
            irr::u32 pixelY_int = round(pixelY_float);

            irr::video::SColor pixelColor = heightMap->getPixel(pixelX_int,pixelY_int);
            
            if (usesRGBEncoding) {
                //Absolute height is (red * 256 + green + blue / 256) - 32768
                heightValue = ((irr::f32)pixelColor.getRed()*256 + (irr::f32)pixelColor.getGreen() + (irr::f32)pixelColor.getBlue()/256.0)-32768.0;
            } else {
                heightValue = pixelColor.getLightness();
            }

            heightMapLine.push_back(heightValue);
        }
        heightMapVector.push_back(heightMapLine);
    }

    return heightMapVector;

}

std::vector<std::vector<irr::f32>> Terrain::heightMapBinaryToVector(irr::io::IReadFile* heightMapFile, irr::u32 binaryWidth, irr::u32 binaryHeight, bool floatingPoint)
{
    std::vector<std::vector<irr::f32>> heightMapVector;

    if (heightMapFile==0) {
        //Return empty vector if we can't load the image
        return heightMapVector;
    }
    
    irr::f32 heightValue;

    for (unsigned int j=0; j<binaryWidth; j++) {
        std::vector<irr::f32> heightMapLine;
        for (unsigned int k=0; k<binaryHeight; k++) {
            
            if (floatingPoint) {
                //read heightValue from binary file: floating point 32 bit
                const size_t bytesPerPixel = 4; //Hard coded for 32 bit (4 byte)
                irr::f32 val;
                if (heightMapFile->read(&val, bytesPerPixel) == bytesPerPixel) {
                    heightMapLine.push_back(val); //We use this as an unscaled height in metres
                } else {
                    heightMapLine.push_back(-1e3); //Fallback, we shouldn't get here?
                }
            } else {
                //read heightValue from binary file: signed 16 bit
                const size_t bytesPerPixel = 2; //Hard coded for 16 bit (2 byte)
                irr::s16 val;
                if (heightMapFile->read(&val, bytesPerPixel) == bytesPerPixel) {
                    heightMapLine.push_back(val); //We use this as an unscaled height in metres
                } else {
                    heightMapLine.push_back(-1e3); //Fallback, we shouldn't get here?
                }
            }


        }
        heightMapVector.push_back(heightMapLine);
    }

    return heightMapVector;

}

void Terrain::addRadarReflectingTerrain(std::vector<std::vector<irr::f32>> heightVector, irr::f32 positionX, irr::f32 positionZ, irr::f32 widthX, irr::f32 widthZ)
{
    //Add a terrain to be used to give the impression of a radar reflection from a land object.
    
    irr::scene::BCTerrainSceneNode* terrain = new irr::scene::BCTerrainSceneNode(
            dev,
            dev->getSceneManager()->getRootSceneNode(), 
            dev->getSceneManager(),
			dev->getSceneManager()->getFileSystem(), -1, 5, irr::scene::ETPS_33
        );

    irr::f32 terrainXLoadScaling = 1;
    irr::f32 terrainZLoadScaling = 1;

    bool loaded = terrain->loadHeightMapVector(heightVector, terrainXLoadScaling, terrainZLoadScaling, irr::video::SColor(255, 255, 255, 255), 0);

    if (!loaded) {
        //Could not load terrain
        std::cerr << "Could not load radar reflecting terrain." << std::endl;
        return;
    }

    irr::f32 scaleX = terrainXLoadScaling*widthX/(terrain->getBoundingBox().MaxEdge.X - terrain->getBoundingBox().MinEdge.X);
    irr::f32 scaleZ = terrainZLoadScaling*widthZ/(terrain->getBoundingBox().MaxEdge.Z - terrain->getBoundingBox().MinEdge.Z);

    terrain->setScale(irr::core::vector3df(scaleX,1.0f,scaleZ));
    terrain->setPosition(irr::core::vector3df(positionX, 0.f, positionZ));
    
    //terrain->getMesh()->getMeshBuffer(0)->getMaterial().setFlag(irr::video::EMF_WIREFRAME, true);
    terrain->setVisible(false);

    terrains.push_back(terrain);
}

std::vector<std::vector<irr::f32>> Terrain::transposeHeightMapVector(std::vector<std::vector<irr::f32>> inVector){
    std::vector<std::vector<irr::f32>> outVector;

    if (inVector.size() == 0 || inVector.at(0).size() == 0) {
		//Zero length, return empty output vector
		return outVector;
	}

    irr::u32 inputHeight = inVector.size();
    irr::u32 inputWidth = inVector.at(0).size();

    for (int i = 0; i < inputWidth; i++) {
        std::vector<irr::f32> lineVector;
        for (int j = 0; j < inputHeight; j++) {
            
            if (j < inVector.size() && i < inVector.at(j).size()) {
                lineVector.push_back(inVector.at(j).at(i));
            } else {
                //If outside the range of the input vector, set a low value
                lineVector.push_back(-1e3);//A big negative value
            }

        }
        outVector.push_back(lineVector);    
    }
    return outVector;
}

std::vector<std::vector<irr::f32>> Terrain::limitSize(std::vector<std::vector<irr::f32>> inVector, irr::u32 maxSize){
    std::vector<std::vector<irr::f32>> outVector;

    if (inVector.size() == 0 || inVector.at(0).size() == 0) {
		//Zero length, return input vector
		return inVector;
	}

    if ((maxSize >= inVector.size()) && (maxSize >= inVector.at(0).size())) {
        //Already within limits, don't need to scale down
		return inVector;    
    }

    irr::u32 inputHeight = inVector.size();
    irr::u32 inputWidth = inVector.at(0).size();

    irr::u32 outputHeight = std::min(inputHeight, maxSize);
    irr::u32 outputWidth = std::min(inputWidth, maxSize);

    for (int i = 0; i < outputHeight; i++) {
        std::vector<irr::f32> lineVector;
        for (int j = 0; j < outputWidth; j++) {
            
            //Pick the pixel to use (very simple scaling, to be replaced with bilinear interpolation)
            irr::f32 pixelX_float = (irr::f32)j * (irr::f32)inputWidth/(irr::f32)outputWidth;
            irr::f32 pixelY_float = (irr::f32)i * (irr::f32)inputHeight/(irr::f32)outputHeight;
            irr::u32 pixelX_int = round(pixelX_float);
            irr::u32 pixelY_int = round(pixelY_float);

            irr::f32 pointValue;
            if ((pixelY_int < inVector.size()) && (pixelX_int < inVector.at(pixelY_int).size())){
                pointValue = inVector.at(pixelY_int).at(pixelX_int);
            } else {
                //Point doesn't exist
                pointValue = -1e3; //A big negative value
            }
            lineVector.push_back(pointValue);
        }
        outVector.push_back(lineVector);    
    }
    return outVector;
}

irr::f32 Terrain::getHeight(irr::f32 x, irr::f32 z) const //Get height from global coordinates
{
    //Fallback minimum value
    irr::f32 terrainHeight = -FLT_MAX;
    
    //Check down list, find highest return value
    for (int i=(int)terrains.size()-1; i>=0; i--) {
        irr::f32 thisHeight = terrains.at(i)->getHeight(x,z);
        if (thisHeight > terrainHeight) {
            terrainHeight = thisHeight;
        }
    }

    return terrainHeight;
}

irr::f32 Terrain::longToX(irr::f32 longitude) const
{
    return ((longitude - primeTerrainLong ) * (primeTerrainXWidth)) / primeTerrainLongExtent;
}

irr::f32 Terrain::latToZ(irr::f32 latitude) const
{
    return ((latitude - primeTerrainLat ) * (primeTerrainZWidth)) / primeTerrainLatExtent;
}

irr::f32 Terrain::xToLong(irr::f32 x) const{
    return primeTerrainLong + x*primeTerrainLongExtent/primeTerrainXWidth;
}

irr::f32 Terrain::zToLat(irr::f32 z) const{
    return primeTerrainLat + z*primeTerrainLatExtent/primeTerrainZWidth;
}

void Terrain::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    for (unsigned int i=0; i<terrains.size(); i++) {
        irr::core::vector3df currentPos = terrains.at(i)->getPosition();
        irr::f32 newPosX = currentPos.X + deltaX;
        irr::f32 newPosY = currentPos.Y + deltaY;
        irr::f32 newPosZ = currentPos.Z + deltaZ;
        terrains.at(i)->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
    }
}
