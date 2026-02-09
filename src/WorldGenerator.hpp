/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

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

#ifndef __WORLDGENERATOR_HPP_INCLUDED__
#define __WORLDGENERATOR_HPP_INCLUDED__

#ifdef WITH_GDAL

#include "ChartReader.hpp"
#include "HeightmapGenerator.hpp"
#include <string>

struct WorldGeneratorResult {
    int buoyCount = 0;
    int lightCount = 0;
    int landmarkCount = 0;
    int depthAreaCount = 0;
    int soundingCount = 0;
    int coastlineCount = 0;
    float maxHeight = 0;
    float maxDepth = 0;
    bool success = false;
};

class WorldGenerator {
public:
    // Generate terrain.ini content from chart data and heightmap stats
    static std::string generateTerrainIni(const HeightmapBounds& bounds,
                                          float maxHeight, float maxDepth,
                                          int heightmapSize,
                                          const std::string& heightmapFilename = "height.png",
                                          const std::string& textureFilename = "texture.png",
                                          const std::string& mapFilename = "map.png");

    // Generate a simple land/sea texture from coastlines (legacy, 2-color)
    // Returns RGB data (3 bytes per pixel), row-major
    static std::vector<uint8_t> generateTexture(const std::vector<CoastlineSegment>& coastlines,
                                                const HeightmapBounds& bounds,
                                                int resolution);

    // Generate a height-aware terrain texture with depth/elevation gradient coloring:
    //   Deep water (>20m):  dark blue (30,60,120)
    //   Medium water (5-20m): blue (40,80,160)
    //   Shallow water (0-5m): light blue (80,140,200)
    //   Intertidal (-1 to 0m): sand (194,178,128)
    //   Low land (0-10m):  dark green (60,120,40)
    //   Mid land (10-50m): medium green (80,140,60)
    //   High land (50m+):  brown/grey (140,130,100)
    //   Urban areas:        grey (160,160,160)
    static std::vector<uint8_t> generateTexture(const std::vector<std::vector<float>>& heightGrid,
                                                const std::vector<UrbanArea>& urbanAreas,
                                                const HeightmapBounds& bounds);

    // Generate a basic map image (same as texture but lower res)
    static std::vector<uint8_t> generateMapImage(const std::vector<CoastlineSegment>& coastlines,
                                                 const HeightmapBounds& bounds,
                                                 int resolution = 512);

    // Generate empty tide/tidal stream ini files
    static std::string generateTideIni();
    static std::string generateTidalStreamIni();

    // Full pipeline: read chart, generate all world files in output directory
    // demDir: optional directory containing Copernicus DEM .tif files for land elevation
    // bathyDir: optional directory containing BlueTopo .tif files for high-res bathymetry
    static WorldGeneratorResult generateWorld(const std::string& chartPath,
                                              const std::string& outputDir,
                                              int heightmapSize = 1025,
                                              const std::string& demDir = "",
                                              const std::string& bathyDir = "");
};

#endif // WITH_GDAL
#endif // __WORLDGENERATOR_HPP_INCLUDED__
