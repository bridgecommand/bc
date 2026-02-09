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

#include "irrlicht.h"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "Utilities.hpp"

#include "BCTerrainSceneNode.h"

#include <iostream>
#include <algorithm>
#include <cmath>

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

void Terrain::load(const std::string& worldPath, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* device, uint32_t terrainResolutionLimit)
{

    dev = device;
    
    irr::video::IVideoDriver* driver = smgr->getVideoDriver();

    //Get full path to the main Terrain.ini file
    std::string worldTerrainFile = worldPath;
    worldTerrainFile.append("/terrain.ini");

    uint32_t numberOfTerrains=0;
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
                for (uint32_t i=0;i<fileList->getFileCount();i++) {
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

        float terrainLong;
        float terrainLat;
        float terrainLongExtent;
        float terrainLatExtent;
        float terrainMaxHeight;
        float seaMaxDepth;
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
            //float terrainHeightMapSize=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapSize",i));

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
        //float scaleX = terrainXWidth / (terrainHeightMapSize);
        float scaleY = (terrainMaxHeight + seaMaxDepth)/ (255.0);
        //float scaleZ = terrainZWidth / (terrainHeightMapSize);
        float terrainY = -1*seaMaxDepth;

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
        float terrainXLoadScaling = 1;
        float terrainZLoadScaling = 1;
        
        //Check extension
        std::string extension = "";
        if (heightMapPath.length() > 3) {
            extension = heightMapPath.substr(heightMapPath.length() - 4,4);
            Utilities::to_lower(extension);
        }
        
        if (extension.compare(".f32") == 0 ) {
            
            //Binary file

            
            uint32_t binaryRows = IniFile::iniFileTou32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapRows",i));
            uint32_t binaryCols = IniFile::iniFileTou32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapColumns",i));
            bool flipRowCol = false;

            //Fall back to TerrainHeightMapSize if not set - legacy file loading, also indicating swapped rows and columns
            if (binaryRows == 0 || binaryCols == 0) {
                binaryRows = IniFile::iniFileTou32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapSize",i));    
                binaryCols = IniFile::iniFileTou32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapSize",i));    
                flipRowCol = true;
            }

            //Load from binary file into a vector, 
            std::vector<std::vector<float>> heightMapVector = heightMapBinaryToVector(heightMapFile,binaryRows,binaryCols,true);
            
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
            uint32_t binaryRows = IniFile::iniFileTou32(heightMapPath, "number_of_rows");
            uint32_t binaryCols = IniFile::iniFileTou32(heightMapPath, "number_of_columns");

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
                    std::vector<std::vector<float>> heightMapVector = heightMapBinaryToVector(heightMapFile,binaryRows,binaryCols,floatingPoint);
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
            std::vector<std::vector<float>> heightMapVector = heightMapImageToVector(heightMapFile,usesRGBEncoding,false,smgr);
            
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
        float terrainXWidth = terrainLongExtent * 2.0 * PI * EARTH_RAD_M * cos( irr::core::degToRad(terrainLat + terrainLatExtent/2.0)) / 360.0;
        float terrainZWidth = terrainLatExtent  * 2.0 * PI * EARTH_RAD_M / 360;

        float scaleX = terrainXLoadScaling*terrainXWidth/(terrain->getBoundingBox().MaxEdge.X - terrain->getBoundingBox().MinEdge.X);
        float scaleZ = terrainZLoadScaling*terrainZWidth/(terrain->getBoundingBox().MaxEdge.Z - terrain->getBoundingBox().MinEdge.Z);
            
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
            float deltaX = (terrainLong - primeTerrainLong) * primeTerrainXWidth / primeTerrainLongExtent;
            float deltaZ = (terrainLat - primeTerrainLat) * primeTerrainZWidth / primeTerrainLatExtent;
            float newPosX = currentPos.X + deltaX;
            float newPosY = currentPos.Y;
            float newPosZ = currentPos.Z + deltaZ;
            terrain->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
        }

        //Create terrain name
        std::string internalName = "Terrain_";
        internalName.append(std::to_string(i - 1));
        terrain->setName(internalName.c_str());
        
        //terrain->setDebugDataVisible(35);
        //terrain->getMesh()->getMeshBuffer(0)->getMaterial().setFlag(irr::video::EMF_WIREFRAME, true);
        terrains.push_back(terrain);

    }


}

std::vector<std::vector<float>> Terrain::heightMapImageToVector(irr::io::IReadFile* heightMapFile, bool usesRGBEncoding, bool normaliseSize, irr::scene::ISceneManager* smgr)
{
    irr::video::IImage* heightMap = smgr->getVideoDriver()->createImageFromFile(heightMapFile);
    std::vector<std::vector<float>> heightMapVector;

    if (heightMap==0) {
        //Return empty vector if we can't load the image
        return heightMapVector;
    }

    uint32_t imageWidth = heightMap->getDimension().Width;
    uint32_t imageHeight = heightMap->getDimension().Height;

    int32_t scaledWidth; 
    int32_t scaledHeight;
    if (normaliseSize) {
        //Find nearest 2^n+1 square size, upscaling if needed. 
        //Subtract 1 and find next power of 2, and add one (we need a size that is (2^n)+1)
        scaledWidth= (int32_t)imageWidth-1;
        scaledHeight = (int32_t)imageHeight-1;
        
        scaledWidth = pow(2.0,ceil(log2(scaledWidth))) + 1;
        scaledHeight = pow(2.0,ceil(log2(scaledHeight))) + 1;
        
        //find largest, returned vector will be square
        scaledWidth = std::max(scaledWidth,scaledHeight);
        scaledHeight = scaledWidth;
    } else {
        scaledWidth= (int32_t)imageWidth;
        scaledHeight = (int32_t)imageHeight;
    }
    
    float heightValue;

    for (unsigned int k=0; k<scaledHeight; k++) {
        std::vector<float> heightMapLine;
        for (unsigned int j=0; j<scaledWidth; j++) {
            
            //Pick the pixel to use (very simple scaling, to be replaced with bilinear interpolation)
            float pixelX_float = (float)j * (float)imageWidth/(float)scaledWidth;
            float pixelY_float = (float)(scaledHeight - k - 1) * (float)imageHeight/(float)scaledHeight;
            uint32_t pixelX_int = round(pixelX_float);
            uint32_t pixelY_int = round(pixelY_float);

            irr::video::SColor pixelColor = heightMap->getPixel(pixelX_int,pixelY_int);
            
            if (usesRGBEncoding) {
                //Absolute height is (red * 256 + green + blue / 256) - 32768
                heightValue = ((float)pixelColor.getRed()*256 + (float)pixelColor.getGreen() + (float)pixelColor.getBlue()/256.0)-32768.0;
            } else {
                heightValue = pixelColor.getLightness();
            }

            heightMapLine.push_back(heightValue);
        }
        heightMapVector.push_back(heightMapLine);
    }

    return heightMapVector;

}

std::vector<std::vector<float>> Terrain::heightMapBinaryToVector(irr::io::IReadFile* heightMapFile, uint32_t binaryWidth, uint32_t binaryHeight, bool floatingPoint)
{
    std::vector<std::vector<float>> heightMapVector;

    if (heightMapFile==0) {
        //Return empty vector if we can't load the image
        return heightMapVector;
    }
    
    float heightValue;

    for (unsigned int j=0; j<binaryWidth; j++) {
        std::vector<float> heightMapLine;
        for (unsigned int k=0; k<binaryHeight; k++) {
            
            if (floatingPoint) {
                //read heightValue from binary file: floating point 32 bit
                const size_t bytesPerPixel = 4; //Hard coded for 32 bit (4 byte)
                float val;
                if (heightMapFile->read(&val, bytesPerPixel) == bytesPerPixel) {
                    if (std::isnormal(val)) {
                        heightMapLine.push_back(val); //We use this as an unscaled height in metres
                    }
                    else {
                        heightMapLine.push_back(-1e3); // If nan or inf, if so, set to -1e3, to indicate missing data
                    }
                } else {
                    heightMapLine.push_back(-1e3); //Fallback, we shouldn't get here?
                }
            } else {
                //read heightValue from binary file: signed 16 bit
                const size_t bytesPerPixel = 2; //Hard coded for 16 bit (2 byte)
                int16_t val;
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

void Terrain::addRadarReflectingTerrain(std::vector<std::vector<float>> heightVector, float positionX, float positionZ, float widthX, float widthZ)
{
    //Add a terrain to be used to give the impression of a radar reflection from a land object.
    
    irr::scene::BCTerrainSceneNode* terrain = new irr::scene::BCTerrainSceneNode(
            dev,
            dev->getSceneManager()->getRootSceneNode(), 
            dev->getSceneManager(),
			dev->getSceneManager()->getFileSystem(), -1, 5, irr::scene::ETPS_33
        );

    float terrainXLoadScaling = 1;
    float terrainZLoadScaling = 1;

    bool loaded = terrain->loadHeightMapVector(heightVector, terrainXLoadScaling, terrainZLoadScaling, irr::video::SColor(255, 255, 255, 255), 0);

    if (!loaded) {
        //Could not load terrain
        std::cerr << "Could not load radar reflecting terrain." << std::endl;
        return;
    }

    float scaleX = terrainXLoadScaling*widthX/(terrain->getBoundingBox().MaxEdge.X - terrain->getBoundingBox().MinEdge.X);
    float scaleZ = terrainZLoadScaling*widthZ/(terrain->getBoundingBox().MaxEdge.Z - terrain->getBoundingBox().MinEdge.Z);

    terrain->setScale(irr::core::vector3df(scaleX,1.0f,scaleZ));
    terrain->setPosition(irr::core::vector3df(positionX, 0.f, positionZ));
    
    //terrain->getMesh()->getMeshBuffer(0)->getMaterial().setFlag(irr::video::EMF_WIREFRAME, true);
    terrain->setVisible(false);

    terrains.push_back(terrain);
}

irr::scene::ISceneNode* Terrain::getSceneNode(int number)
{
    if (number < (int)terrains.size() && number >= 0) {
        return terrains.at(number);
    }
    else {
        return 0;
    }
}

std::vector<std::vector<float>> Terrain::transposeHeightMapVector(std::vector<std::vector<float>> inVector){
    std::vector<std::vector<float>> outVector;

    if (inVector.size() == 0 || inVector.at(0).size() == 0) {
		//Zero length, return empty output vector
		return outVector;
	}

    uint32_t inputHeight = inVector.size();
    uint32_t inputWidth = inVector.at(0).size();

    for (int i = 0; i < inputWidth; i++) {
        std::vector<float> lineVector;
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

std::vector<std::vector<float>> Terrain::limitSize(std::vector<std::vector<float>> inVector, uint32_t maxSize){
    std::vector<std::vector<float>> outVector;

    if (inVector.size() == 0 || inVector.at(0).size() == 0) {
		//Zero length, return input vector
		return inVector;
	}

    if ((maxSize >= inVector.size()) && (maxSize >= inVector.at(0).size())) {
        //Already within limits, don't need to scale down
		return inVector;    
    }

    uint32_t inputHeight = inVector.size();
    uint32_t inputWidth = inVector.at(0).size();

    uint32_t outputHeight = std::min(inputHeight, maxSize);
    uint32_t outputWidth = std::min(inputWidth, maxSize);

    for (int i = 0; i < outputHeight; i++) {
        std::vector<float> lineVector;
        for (int j = 0; j < outputWidth; j++) {
            
            //Pick the pixel to use (very simple scaling, to be replaced with bilinear interpolation)
            float pixelX_float = (float)j * (float)inputWidth/(float)outputWidth;
            float pixelY_float = (float)i * (float)inputHeight/(float)outputHeight;
            uint32_t pixelX_int = round(pixelX_float);
            uint32_t pixelY_int = round(pixelY_float);

            float pointValue;
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

float Terrain::getHeight(float x, float z) const //Get height from global coordinates
{
    //Fallback minimum value
    float terrainHeight = -FLT_MAX;
    
    //Check down list, find highest return value
    for (int i=(int)terrains.size()-1; i>=0; i--) {
        float thisHeight = terrains.at(i)->getHeight(x,z);
        if (thisHeight > terrainHeight) {
            terrainHeight = thisHeight;
        }
    }

    return terrainHeight;
}

float Terrain::longToX(float longitude) const
{
    return ((longitude - primeTerrainLong ) * (primeTerrainXWidth)) / primeTerrainLongExtent;
}

float Terrain::latToZ(float latitude) const
{
    return ((latitude - primeTerrainLat ) * (primeTerrainZWidth)) / primeTerrainLatExtent;
}

float Terrain::xToLong(float x) const{
    return primeTerrainLong + x*primeTerrainLongExtent/primeTerrainXWidth;
}

float Terrain::zToLat(float z) const{
    return primeTerrainLat + z*primeTerrainLatExtent/primeTerrainZWidth;
}

void Terrain::moveNode(float deltaX, float deltaY, float deltaZ)
{
    for (unsigned int i=0; i<terrains.size(); i++) {
        irr::core::vector3df currentPos = terrains.at(i)->getPosition();
        float newPosX = currentPos.X + deltaX;
        float newPosY = currentPos.Y + deltaY;
        float newPosZ = currentPos.Z + deltaZ;
        terrains.at(i)->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
    }
}
