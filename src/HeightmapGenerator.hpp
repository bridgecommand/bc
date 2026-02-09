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

#ifndef __HEIGHTMAPGENERATOR_HPP_INCLUDED__
#define __HEIGHTMAPGENERATOR_HPP_INCLUDED__

#ifdef WITH_GDAL

#include "ChartReader.hpp"
#include <string>
#include <vector>
#include <memory>

struct HeightmapBounds {
    double minLon, maxLon;
    double minLat, maxLat;
};

struct HeightmapParams {
    int resolution = 1025;           // Grid size (pixels per side)
    double defaultLandHeight = 5.0;  // Default land elevation (metres) when no DEM
    double defaultSeaDepth = 0.0;    // Default depth for uncharted water
    int idwNeighbours = 8;           // Neighbours for IDW sounding interpolation
    double idwPower = 2.0;           // IDW distance power exponent
};

class HeightmapGenerator {
public:
    HeightmapGenerator();

    // Set chart data for generation
    void setDepthAreas(const std::vector<DepthArea>& areas);
    void setSoundings(const std::vector<Sounding>& soundings);
    void setCoastlines(const std::vector<CoastlineSegment>& coastlines);

    // Load DEM GeoTIFF files for land elevation (e.g. Copernicus DEM tiles)
    // Multiple tiles are merged to cover the full area. Requires GDAL.
    bool loadDEMTiles(const std::vector<std::string>& tifPaths);
    bool loadDEMTile(const std::string& tifPath);

    // Load high-resolution bathymetry GeoTIFF files (e.g. BlueTopo COG)
    // When available, these override S-57 DEPARE polygon depths for water pixels.
    // Values in GeoTIFF should be negative (below sea level) or positive (land).
    bool loadBathymetryTiles(const std::vector<std::string>& tifPaths);
    bool loadBathymetryTile(const std::string& tifPath);

    // Set bounding box (auto-computed from data if not set)
    void setBounds(const HeightmapBounds& bounds);
    HeightmapBounds computeBoundsFromData() const;

    // Generate the height grid (metres, positive up, negative = underwater)
    // Returns a row-major 2D grid [row][col] of float heights
    std::vector<std::vector<float>> generate(const HeightmapParams& params = HeightmapParams{}) const;

    // Encode height grid to Bridge Command RGB format (R*256 + G + B/256 - 32768)
    // Returns RGB pixel data (3 bytes per pixel), row-major
    static std::vector<uint8_t> encodeRGB(const std::vector<std::vector<float>>& heightGrid);

    // Write RGB data as PNG file
    static bool writePNG(const std::string& path, const std::vector<uint8_t>& rgbData,
                         int width, int height);

    // Convenience: generate and write in one step
    bool generateAndWrite(const std::string& outputPath,
                          const HeightmapParams& params = HeightmapParams{}) const;

    // Get stats from last generation
    float getMaxHeight() const { return maxHeight; }
    float getMaxDepth() const { return maxDepth; }

    // Point-in-polygon test (ray casting algorithm)
    static bool pointInPolygon(double x, double y, const std::vector<ChartPoint>& polygon);

private:
    std::vector<DepthArea> depthAreas;
    std::vector<Sounding> soundings;
    std::vector<CoastlineSegment> coastlines;
    HeightmapBounds bounds;
    bool boundsSet = false;

    mutable float maxHeight = 0;
    mutable float maxDepth = 0;

    // Loaded DEM tile data
    struct DEMTile {
        std::vector<float> data;   // Row-major elevation data
        int width = 0, height = 0;
        double originLon = 0, originLat = 0; // Top-left corner
        double pixelSizeLon = 0, pixelSizeLat = 0; // Degrees per pixel (lat is negative)
    };
    std::vector<DEMTile> demTiles;
    std::vector<DEMTile> bathymetryTiles;

    // Load a GeoTIFF into a DEMTile struct (shared implementation)
    static bool loadGeoTIFF(const std::string& tifPath, DEMTile& tile);

    // Sample elevation from a set of tiles at a given lon/lat
    // Returns NaN if no coverage at that point
    static float sampleTiles(const std::vector<DEMTile>& tiles, double lon, double lat);

public:
    // Sample from DEM tiles (land elevation)
    float sampleDEM(double lon, double lat) const;

    // Sample from bathymetry tiles (underwater depth)
    float sampleBathymetry(double lon, double lat) const;

private:

    // IDW interpolation from sounding points
    float interpolateSoundings(double lon, double lat, const HeightmapParams& params) const;

    // Check if a point is on land (inside any coastline/land polygon)
    bool isLand(double lon, double lat) const;
};

#endif // WITH_GDAL
#endif // __HEIGHTMAPGENERATOR_HPP_INCLUDED__
