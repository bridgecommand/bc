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

#include "HeightmapGenerator.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb/stb_image_write.h"

#include "gdal_priv.h"

#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream>

HeightmapGenerator::HeightmapGenerator() : bounds{0,0,0,0} {
}

void HeightmapGenerator::setDepthAreas(const std::vector<DepthArea>& areas) {
    depthAreas = areas;
}

void HeightmapGenerator::setSoundings(const std::vector<Sounding>& s) {
    soundings = s;
}

void HeightmapGenerator::setCoastlines(const std::vector<CoastlineSegment>& c) {
    coastlines = c;
}

// ── GeoTIFF loading (shared by DEM and bathymetry) ─────────────────────────

bool HeightmapGenerator::loadGeoTIFF(const std::string& tifPath, DEMTile& tile) {
    GDALAllRegister();

    GDALDataset* ds = static_cast<GDALDataset*>(GDALOpen(tifPath.c_str(), GA_ReadOnly));
    if (!ds) {
        std::cerr << "HeightmapGenerator: Failed to open " << tifPath << std::endl;
        return false;
    }

    GDALRasterBand* band = ds->GetRasterBand(1);
    if (!band) {
        std::cerr << "HeightmapGenerator: No raster band in " << tifPath << std::endl;
        GDALClose(ds);
        return false;
    }

    tile.width = ds->GetRasterXSize();
    tile.height = ds->GetRasterYSize();

    double geoTransform[6];
    if (ds->GetGeoTransform(geoTransform) != CE_None) {
        std::cerr << "HeightmapGenerator: No geotransform in " << tifPath << std::endl;
        GDALClose(ds);
        return false;
    }

    tile.originLon = geoTransform[0];
    tile.pixelSizeLon = geoTransform[1];
    tile.originLat = geoTransform[3];
    tile.pixelSizeLat = geoTransform[5]; // Negative for north-up

    tile.data.resize(static_cast<size_t>(tile.width) * tile.height);
    CPLErr err = band->RasterIO(GF_Read, 0, 0, tile.width, tile.height,
                                 tile.data.data(), tile.width, tile.height,
                                 GDT_Float32, 0, 0);
    if (err != CE_None) {
        std::cerr << "HeightmapGenerator: Failed to read raster from " << tifPath << std::endl;
        GDALClose(ds);
        return false;
    }

    int hasNoData = 0;
    double nodata = band->GetNoDataValue(&hasNoData);
    if (hasNoData) {
        for (auto& v : tile.data) {
            if (v == static_cast<float>(nodata)) {
                v = std::numeric_limits<float>::quiet_NaN();
            }
        }
    }

    GDALClose(ds);
    return true;
}

float HeightmapGenerator::sampleTiles(const std::vector<DEMTile>& tiles,
                                       double lon, double lat) {
    for (const auto& tile : tiles) {
        double col = (lon - tile.originLon) / tile.pixelSizeLon;
        double row = (lat - tile.originLat) / tile.pixelSizeLat;

        int c = static_cast<int>(col);
        int r = static_cast<int>(row);

        if (c < 0 || c >= tile.width - 1 || r < 0 || r >= tile.height - 1)
            continue;

        float fCol = static_cast<float>(col - c);
        float fRow = static_cast<float>(row - r);

        float v00 = tile.data[r * tile.width + c];
        float v10 = tile.data[r * tile.width + (c + 1)];
        float v01 = tile.data[(r + 1) * tile.width + c];
        float v11 = tile.data[(r + 1) * tile.width + (c + 1)];

        if (std::isnan(v00) || std::isnan(v10) || std::isnan(v01) || std::isnan(v11)) {
            int nr = static_cast<int>(row + 0.5);
            int nc = static_cast<int>(col + 0.5);
            if (nr >= 0 && nr < tile.height && nc >= 0 && nc < tile.width) {
                float nearest = tile.data[nr * tile.width + nc];
                if (!std::isnan(nearest)) return nearest;
            }
            continue;
        }

        float top = v00 + (v10 - v00) * fCol;
        float bottom = v01 + (v11 - v01) * fCol;
        return top + (bottom - top) * fRow;
    }

    return std::numeric_limits<float>::quiet_NaN();
}

// ── DEM tile loading (land elevation) ──────────────────────────────────────

bool HeightmapGenerator::loadDEMTile(const std::string& tifPath) {
    DEMTile tile;
    if (!loadGeoTIFF(tifPath, tile)) return false;
    std::cout << "HeightmapGenerator: Loaded DEM tile " << tifPath
              << " (" << tile.width << "x" << tile.height << ")" << std::endl;
    demTiles.push_back(std::move(tile));
    return true;
}

bool HeightmapGenerator::loadDEMTiles(const std::vector<std::string>& tifPaths) {
    bool anyLoaded = false;
    for (const auto& path : tifPaths) {
        if (loadDEMTile(path)) anyLoaded = true;
    }
    return anyLoaded;
}

float HeightmapGenerator::sampleDEM(double lon, double lat) const {
    return sampleTiles(demTiles, lon, lat);
}

// ── Bathymetry tile loading (underwater depth - e.g. BlueTopo) ─────────────

bool HeightmapGenerator::loadBathymetryTile(const std::string& tifPath) {
    DEMTile tile;
    if (!loadGeoTIFF(tifPath, tile)) return false;
    std::cout << "HeightmapGenerator: Loaded bathymetry tile " << tifPath
              << " (" << tile.width << "x" << tile.height << ")" << std::endl;
    bathymetryTiles.push_back(std::move(tile));
    return true;
}

bool HeightmapGenerator::loadBathymetryTiles(const std::vector<std::string>& tifPaths) {
    bool anyLoaded = false;
    for (const auto& path : tifPaths) {
        if (loadBathymetryTile(path)) anyLoaded = true;
    }
    return anyLoaded;
}

float HeightmapGenerator::sampleBathymetry(double lon, double lat) const {
    return sampleTiles(bathymetryTiles, lon, lat);
}

void HeightmapGenerator::setBounds(const HeightmapBounds& b) {
    bounds = b;
    boundsSet = true;
}

HeightmapBounds HeightmapGenerator::computeBoundsFromData() const {
    HeightmapBounds b;
    b.minLon = std::numeric_limits<double>::max();
    b.maxLon = std::numeric_limits<double>::lowest();
    b.minLat = std::numeric_limits<double>::max();
    b.maxLat = std::numeric_limits<double>::lowest();

    auto expand = [&](double lon, double lat) {
        b.minLon = std::min(b.minLon, lon);
        b.maxLon = std::max(b.maxLon, lon);
        b.minLat = std::min(b.minLat, lat);
        b.maxLat = std::max(b.maxLat, lat);
    };

    for (const auto& area : depthAreas) {
        for (const auto& pt : area.boundary) {
            expand(pt.longitude, pt.latitude);
        }
    }

    for (const auto& s : soundings) {
        expand(s.longitude, s.latitude);
    }

    for (const auto& seg : coastlines) {
        for (const auto& pt : seg.points) {
            expand(pt.longitude, pt.latitude);
        }
    }

    // Add small margin (1% on each side)
    double lonMargin = (b.maxLon - b.minLon) * 0.01;
    double latMargin = (b.maxLat - b.minLat) * 0.01;
    if (lonMargin < 0.001) lonMargin = 0.001;
    if (latMargin < 0.001) latMargin = 0.001;
    b.minLon -= lonMargin;
    b.maxLon += lonMargin;
    b.minLat -= latMargin;
    b.maxLat += latMargin;

    return b;
}

// Ray casting algorithm for point-in-polygon
bool HeightmapGenerator::pointInPolygon(double x, double y,
                                         const std::vector<ChartPoint>& polygon) {
    if (polygon.size() < 3) return false;

    bool inside = false;
    size_t n = polygon.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        double xi = polygon[i].longitude, yi = polygon[i].latitude;
        double xj = polygon[j].longitude, yj = polygon[j].latitude;

        if (((yi > y) != (yj > y)) &&
            (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
            inside = !inside;
        }
    }
    return inside;
}

float HeightmapGenerator::interpolateSoundings(double lon, double lat,
                                                const HeightmapParams& params) const {
    if (soundings.empty()) return std::numeric_limits<float>::quiet_NaN();

    // Collect distances and weights for IDW
    struct DistVal {
        double dist;
        double depth;
    };

    std::vector<DistVal> candidates;
    candidates.reserve(soundings.size());

    // Approximate metres per degree at this latitude
    double cosLat = cos(lat * M_PI / 180.0);
    double mPerDegLat = 111320.0;
    double mPerDegLon = 111320.0 * cosLat;

    for (const auto& s : soundings) {
        double dx = (s.longitude - lon) * mPerDegLon;
        double dy = (s.latitude - lat) * mPerDegLat;
        double dist = sqrt(dx * dx + dy * dy);
        if (dist < 0.1) {
            // Very close to an exact sounding - use it directly
            return static_cast<float>(-s.depth); // Negative because depth is positive down
        }
        candidates.push_back({dist, s.depth});
    }

    // Sort by distance and take nearest N
    std::sort(candidates.begin(), candidates.end(),
              [](const DistVal& a, const DistVal& b) { return a.dist < b.dist; });

    int count = std::min(params.idwNeighbours, static_cast<int>(candidates.size()));
    if (count == 0) return std::numeric_limits<float>::quiet_NaN();

    double weightSum = 0;
    double valueSum = 0;
    for (int i = 0; i < count; i++) {
        double w = 1.0 / pow(candidates[i].dist, params.idwPower);
        weightSum += w;
        valueSum += w * candidates[i].depth;
    }

    if (weightSum == 0) return std::numeric_limits<float>::quiet_NaN();
    return static_cast<float>(-(valueSum / weightSum)); // Negative = below water
}

bool HeightmapGenerator::isLand(double lon, double lat) const {
    // Check if point is inside any land polygon (from LNDARE via coastlines)
    // Coastlines from LNDARE are closed polygons representing land
    for (const auto& seg : coastlines) {
        if (seg.points.size() >= 3) {
            // Check if the first and last points are the same (closed polygon = land area)
            const auto& first = seg.points.front();
            const auto& last = seg.points.back();
            double closeDist = fabs(first.longitude - last.longitude) +
                               fabs(first.latitude - last.latitude);
            if (closeDist < 0.0001) {
                // Closed polygon - treat as land area
                if (pointInPolygon(lon, lat, seg.points)) {
                    return true;
                }
            }
        }
    }
    return false;
}

std::vector<std::vector<float>> HeightmapGenerator::generate(
    const HeightmapParams& params) const {

    HeightmapBounds b = boundsSet ? bounds : computeBoundsFromData();

    int res = params.resolution;
    // Initialize grid with NaN (unset)
    std::vector<std::vector<float>> grid(res,
        std::vector<float>(res, std::numeric_limits<float>::quiet_NaN()));

    double lonStep = (b.maxLon - b.minLon) / (res - 1);
    double latStep = (b.maxLat - b.minLat) / (res - 1);

    maxHeight = 0;
    maxDepth = 0;

    // Pass 1: Rasterize depth area polygons
    for (int row = 0; row < res; row++) {
        double lat = b.maxLat - row * latStep; // Top to bottom
        for (int col = 0; col < res; col++) {
            double lon = b.minLon + col * lonStep;

            // Check each depth area (last match wins - deeper areas typically overlay)
            for (const auto& area : depthAreas) {
                if (area.boundary.size() >= 3 &&
                    pointInPolygon(lon, lat, area.boundary)) {
                    // Use average of min/max depth, negative = below water
                    float depth = -static_cast<float>((area.minDepth + area.maxDepth) / 2.0);
                    grid[row][col] = depth;
                }
            }
        }
    }

    // Pass 2: Refine with sounding interpolation where available
    if (!soundings.empty()) {
        for (int row = 0; row < res; row++) {
            double lat = b.maxLat - row * latStep;
            for (int col = 0; col < res; col++) {
                double lon = b.minLon + col * lonStep;

                // Only interpolate in water areas (already set from depth areas or NaN)
                if (grid[row][col] <= 0 || std::isnan(grid[row][col])) {
                    float interpolated = interpolateSoundings(lon, lat, params);
                    if (!std::isnan(interpolated)) {
                        // Blend: if depth area value exists, average with interpolated
                        if (!std::isnan(grid[row][col]) && grid[row][col] < 0) {
                            grid[row][col] = (grid[row][col] + interpolated) / 2.0f;
                        } else if (std::isnan(grid[row][col])) {
                            grid[row][col] = interpolated;
                        }
                    }
                }
            }
        }
    }

    // Pass 2b: Override with high-resolution bathymetry (BlueTopo) where available
    if (!bathymetryTiles.empty()) {
        for (int row = 0; row < res; row++) {
            double lat = b.maxLat - row * latStep;
            for (int col = 0; col < res; col++) {
                double lon = b.minLon + col * lonStep;

                float bathyVal = sampleBathymetry(lon, lat);
                if (!std::isnan(bathyVal) && bathyVal < 0.0f) {
                    // BlueTopo provides negative values for underwater,
                    // which matches our convention. Override chart data.
                    grid[row][col] = bathyVal;
                }
            }
        }
    }

    // Pass 3: Set land areas (using DEM if available, otherwise defaultLandHeight)
    bool haveDEM = !demTiles.empty();
    for (int row = 0; row < res; row++) {
        double lat = b.maxLat - row * latStep;
        for (int col = 0; col < res; col++) {
            double lon = b.minLon + col * lonStep;

            if (isLand(lon, lat)) {
                if (haveDEM) {
                    float demHeight = sampleDEM(lon, lat);
                    if (!std::isnan(demHeight) && demHeight >= 0.0f) {
                        grid[row][col] = demHeight;
                    } else {
                        grid[row][col] = static_cast<float>(params.defaultLandHeight);
                    }
                } else {
                    grid[row][col] = static_cast<float>(params.defaultLandHeight);
                }
            }

            // Fill remaining NaN with default sea depth
            if (std::isnan(grid[row][col])) {
                grid[row][col] = -static_cast<float>(params.defaultSeaDepth);
            }

            // Track stats
            if (grid[row][col] > maxHeight) maxHeight = grid[row][col];
            if (grid[row][col] < -maxDepth) maxDepth = -grid[row][col];
        }
    }

    return grid;
}

std::vector<uint8_t> HeightmapGenerator::encodeRGB(
    const std::vector<std::vector<float>>& heightGrid) {

    if (heightGrid.empty()) return {};

    int rows = static_cast<int>(heightGrid.size());
    int cols = static_cast<int>(heightGrid[0].size());
    std::vector<uint8_t> rgb(rows * cols * 3);

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            // Bridge Command format: Height = R*256 + G + B/256 - 32768
            // So: encoded = Height + 32768
            // R = encoded / 256 (integer part)
            // G = encoded % 256 (integer remainder)
            // B = fractional * 256
            float height = heightGrid[row][col];
            double encoded = static_cast<double>(height) + 32768.0;

            // Clamp to valid range (0 to 65535.996)
            encoded = std::max(0.0, std::min(encoded, 65535.996));

            int intPart = static_cast<int>(encoded);
            double frac = encoded - intPart;

            uint8_t r = static_cast<uint8_t>(intPart / 256);
            uint8_t g = static_cast<uint8_t>(intPart % 256);
            uint8_t b = static_cast<uint8_t>(frac * 256.0);

            int idx = (row * cols + col) * 3;
            rgb[idx + 0] = r;
            rgb[idx + 1] = g;
            rgb[idx + 2] = b;
        }
    }

    return rgb;
}

bool HeightmapGenerator::writePNG(const std::string& path,
                                   const std::vector<uint8_t>& rgbData,
                                   int width, int height) {
    if (rgbData.size() != static_cast<size_t>(width * height * 3)) {
        std::cerr << "HeightmapGenerator: RGB data size mismatch" << std::endl;
        return false;
    }

    int result = stbi_write_png(path.c_str(), width, height, 3,
                                rgbData.data(), width * 3);
    return result != 0;
}

bool HeightmapGenerator::generateAndWrite(const std::string& outputPath,
                                           const HeightmapParams& params) const {
    auto grid = generate(params);
    if (grid.empty()) return false;

    auto rgb = encodeRGB(grid);
    return writePNG(outputPath, rgb, params.resolution, params.resolution);
}

#endif // WITH_GDAL
