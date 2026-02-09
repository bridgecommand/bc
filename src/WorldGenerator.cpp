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

#ifdef WITH_GDAL

#include "WorldGenerator.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

std::string WorldGenerator::generateTerrainIni(const HeightmapBounds& bounds,
                                               float maxHeight, float maxDepth,
                                               int heightmapSize,
                                               const std::string& heightmapFilename,
                                               const std::string& textureFilename,
                                               const std::string& mapFilename) {
    std::ostringstream oss;
    oss.precision(14);
    oss << std::fixed;

    double lonExtent = bounds.maxLon - bounds.minLon;
    double latExtent = bounds.maxLat - bounds.minLat;

    // Ensure minimum values for max height / depth
    if (maxHeight < 1.0f) maxHeight = 5.0f;
    if (maxDepth < 1.0f) maxDepth = 10.0f;

    oss << "Number=1\n";
    oss << "MapImage=" << mapFilename << "\n";
    oss << "HeightMap(1)=" << heightmapFilename << "\n";
    oss << "Texture(1)=" << textureFilename << "\n";
    oss << "TerrainLong(1)=" << bounds.minLon << "\n";
    oss << "TerrainLat(1)=" << bounds.minLat << "\n";
    oss << "TerrainLongExtent(1)=" << lonExtent << "\n";
    oss << "TerrainLatExtent(1)=" << latExtent << "\n";
    oss << "TerrainHeightMapRows(1)=" << heightmapSize << "\n";
    oss << "TerrainHeightMapColumns(1)=" << heightmapSize << "\n";

    oss.precision(5);
    oss << "TerrainMaxHeight(1)=" << maxHeight << "\n";
    oss << "SeaMaxDepth(1)=" << maxDepth << "\n";
    oss << "UsesRGB(1)=1\n";

    return oss.str();
}

std::vector<uint8_t> WorldGenerator::generateTexture(
    const std::vector<CoastlineSegment>& coastlines,
    const HeightmapBounds& bounds, int resolution) {

    std::vector<uint8_t> rgb(resolution * resolution * 3);

    double lonStep = (bounds.maxLon - bounds.minLon) / (resolution - 1);
    double latStep = (bounds.maxLat - bounds.minLat) / (resolution - 1);

    // Default everything to sea blue
    for (int i = 0; i < resolution * resolution; i++) {
        rgb[i * 3 + 0] = 40;   // R - dark blue
        rgb[i * 3 + 1] = 80;   // G
        rgb[i * 3 + 2] = 140;  // B
    }

    // Fill land areas with green
    for (int row = 0; row < resolution; row++) {
        double lat = bounds.maxLat - row * latStep;
        for (int col = 0; col < resolution; col++) {
            double lon = bounds.minLon + col * lonStep;

            // Check if inside any closed coastline polygon
            for (const auto& seg : coastlines) {
                if (seg.points.size() >= 3) {
                    const auto& first = seg.points.front();
                    const auto& last = seg.points.back();
                    double closeDist = fabs(first.longitude - last.longitude) +
                                       fabs(first.latitude - last.latitude);
                    if (closeDist < 0.0001) {
                        if (HeightmapGenerator::pointInPolygon(lon, lat, seg.points)) {
                            int idx = (row * resolution + col) * 3;
                            rgb[idx + 0] = 60;   // R - dark green
                            rgb[idx + 1] = 120;  // G
                            rgb[idx + 2] = 40;   // B
                            break;
                        }
                    }
                }
            }
        }
    }

    return rgb;
}

std::vector<uint8_t> WorldGenerator::generateTexture(
    const std::vector<std::vector<float>>& heightGrid,
    const std::vector<UrbanArea>& urbanAreas,
    const HeightmapBounds& bounds) {

    if (heightGrid.empty() || heightGrid[0].empty()) return {};

    int rows = static_cast<int>(heightGrid.size());
    int cols = static_cast<int>(heightGrid[0].size());
    std::vector<uint8_t> rgb(rows * cols * 3);

    double lonStep = (bounds.maxLon - bounds.minLon) / (cols - 1);
    double latStep = (bounds.maxLat - bounds.minLat) / (rows - 1);

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            float h = heightGrid[row][col];
            int idx = (row * cols + col) * 3;
            uint8_t r, g, b;

            if (h < -20.0f) {
                // Deep water: dark blue
                r = 30; g = 60; b = 120;
            } else if (h < -5.0f) {
                // Medium water: blue
                r = 40; g = 80; b = 160;
            } else if (h < -0.5f) {
                // Shallow water: light blue
                r = 80; g = 140; b = 200;
            } else if (h < 0.5f) {
                // Intertidal: sand
                r = 194; g = 178; b = 128;
            } else if (h < 10.0f) {
                // Low land: dark green
                r = 60; g = 120; b = 40;
            } else if (h < 50.0f) {
                // Mid land: medium green
                r = 80; g = 140; b = 60;
            } else {
                // High land: brown/grey
                r = 140; g = 130; b = 100;
            }

            rgb[idx + 0] = r;
            rgb[idx + 1] = g;
            rgb[idx + 2] = b;
        }
    }

    // Overlay urban areas with grey
    for (const auto& ua : urbanAreas) {
        if (ua.boundary.size() < 3) continue;
        for (int row = 0; row < rows; row++) {
            double lat = bounds.maxLat - row * latStep;
            for (int col = 0; col < cols; col++) {
                double lon = bounds.minLon + col * lonStep;
                if (HeightmapGenerator::pointInPolygon(lon, lat, ua.boundary)) {
                    float h = heightGrid[row][col];
                    // Only colour land pixels as urban (not water)
                    if (h >= 0.0f) {
                        int idx = (row * cols + col) * 3;
                        rgb[idx + 0] = 160;
                        rgb[idx + 1] = 160;
                        rgb[idx + 2] = 160;
                    }
                }
            }
        }
    }

    return rgb;
}

std::vector<uint8_t> WorldGenerator::generateMapImage(
    const std::vector<CoastlineSegment>& coastlines,
    const HeightmapBounds& bounds, int resolution) {
    // Map image is the same as texture but lower resolution
    return generateTexture(coastlines, bounds, resolution);
}

std::string WorldGenerator::generateTideIni() {
    return "Number=0\n";
}

std::string WorldGenerator::generateTidalStreamIni() {
    return "Number=0\n";
}

WorldGeneratorResult WorldGenerator::generateWorld(const std::string& chartPath,
                                                    const std::string& outputDir,
                                                    int heightmapSize,
                                                    const std::string& demDir,
                                                    const std::string& bathyDir) {
    WorldGeneratorResult result;

    // Open chart
    ChartReader reader;
    if (!reader.open(chartPath)) {
        std::cerr << "WorldGenerator: Failed to open chart " << chartPath << std::endl;
        return result;
    }

    // Extract all features
    auto buoys = reader.extractBuoys();
    auto lights = reader.extractLights();
    auto landmarks = reader.extractLandmarks();
    auto depthAreas = reader.extractDepthAreas();
    auto soundings = reader.extractSoundings();
    auto coastlines = reader.extractCoastlines();
    auto urbanAreas = reader.extractUrbanAreas();

    result.buoyCount = static_cast<int>(buoys.size());
    result.lightCount = static_cast<int>(lights.size());
    result.landmarkCount = static_cast<int>(landmarks.size());
    result.depthAreaCount = static_cast<int>(depthAreas.size());
    result.soundingCount = static_cast<int>(soundings.size());
    result.coastlineCount = static_cast<int>(coastlines.size());

    // Generate heightmap
    HeightmapGenerator hmGen;
    hmGen.setDepthAreas(depthAreas);
    hmGen.setSoundings(soundings);
    hmGen.setCoastlines(coastlines);

    HeightmapBounds bounds = hmGen.computeBoundsFromData();
    hmGen.setBounds(bounds);

    // Load DEM tiles for land elevation (if directory provided)
    if (!demDir.empty()) {
        // Find all .tif files in the DEM directory
        std::vector<std::string> demPaths;
        // Use GDAL's directory listing or simple approach
#ifdef _WIN32
        WIN32_FIND_DATA fd;
        std::string pattern = demDir + "/*.tif";
        HANDLE hFind = FindFirstFile(pattern.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                demPaths.push_back(demDir + "/" + fd.cFileName);
            } while (FindNextFile(hFind, &fd));
            FindClose(hFind);
        }
#else
        // POSIX directory listing
        DIR* dir = opendir(demDir.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name.size() > 4 && name.substr(name.size() - 4) == ".tif") {
                    demPaths.push_back(demDir + "/" + name);
                }
            }
            closedir(dir);
        }
#endif
        if (!demPaths.empty()) {
            std::cout << "WorldGenerator: Loading " << demPaths.size()
                      << " DEM tile(s) from " << demDir << std::endl;
            hmGen.loadDEMTiles(demPaths);
        } else {
            std::cerr << "WorldGenerator: No .tif files found in " << demDir << std::endl;
        }
    }

    // Load BlueTopo bathymetry tiles (if directory provided)
    if (!bathyDir.empty()) {
        std::vector<std::string> bathyPaths;
#ifdef _WIN32
        WIN32_FIND_DATA fd;
        std::string pattern = bathyDir + "/*.tif";
        HANDLE hFind = FindFirstFile(pattern.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                bathyPaths.push_back(bathyDir + "/" + fd.cFileName);
            } while (FindNextFile(hFind, &fd));
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(bathyDir.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name.size() > 4 && name.substr(name.size() - 4) == ".tif") {
                    bathyPaths.push_back(bathyDir + "/" + name);
                }
            }
            closedir(dir);
        }
#endif
        if (!bathyPaths.empty()) {
            std::cout << "WorldGenerator: Loading " << bathyPaths.size()
                      << " bathymetry tile(s) from " << bathyDir << std::endl;
            hmGen.loadBathymetryTiles(bathyPaths);
        } else {
            std::cerr << "WorldGenerator: No .tif files found in " << bathyDir << std::endl;
        }
    }

    HeightmapParams params;
    params.resolution = heightmapSize;

    // Generate height grid (used for both heightmap PNG and texture)
    auto heightGrid = hmGen.generate(params);
    if (heightGrid.empty()) {
        std::cerr << "WorldGenerator: Failed to generate heightmap" << std::endl;
        return result;
    }

    // Write heightmap PNG
    auto rgbData = HeightmapGenerator::encodeRGB(heightGrid);
    if (!HeightmapGenerator::writePNG(outputDir + "/height.png", rgbData,
                                       heightmapSize, heightmapSize)) {
        std::cerr << "WorldGenerator: Failed to write height.png" << std::endl;
        return result;
    }

    result.maxHeight = hmGen.getMaxHeight();
    result.maxDepth = hmGen.getMaxDepth();

    // Write terrain.ini
    std::string terrainIni = generateTerrainIni(bounds, result.maxHeight, result.maxDepth,
                                                heightmapSize);
    {
        std::ofstream f(outputDir + "/terrain.ini");
        if (!f.is_open()) {
            std::cerr << "WorldGenerator: Failed to write terrain.ini" << std::endl;
            return result;
        }
        f << terrainIni;
    }

    // Write height-aware texture and simple map image
    auto textureData = generateTexture(heightGrid, urbanAreas, bounds);
    HeightmapGenerator::writePNG(outputDir + "/texture.png", textureData,
                                 heightmapSize, heightmapSize);

    auto mapData = generateMapImage(coastlines, bounds, 512);
    HeightmapGenerator::writePNG(outputDir + "/map.png", mapData, 512, 512);

    // Write buoy.ini
    {
        std::ofstream f(outputDir + "/buoy.ini");
        if (f.is_open()) f << ChartReader::generateBuoyIni(buoys);
    }

    // Write light.ini
    {
        std::ofstream f(outputDir + "/light.ini");
        if (f.is_open()) f << ChartReader::generateLightIni(buoys, lights);
    }

    // Write landobject.ini
    {
        std::ofstream f(outputDir + "/landobject.ini");
        if (f.is_open()) f << ChartReader::generateLandObjectIni(landmarks);
    }

    // Write tide.ini and tidalstream.ini (empty defaults)
    {
        std::ofstream f(outputDir + "/tide.ini");
        if (f.is_open()) f << generateTideIni();
    }
    {
        std::ofstream f(outputDir + "/tidalstream.ini");
        if (f.is_open()) f << generateTidalStreamIni();
    }

    result.success = true;
    return result;
}

#endif // WITH_GDAL
