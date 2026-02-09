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

// bc-chart-converter: Converts S-57 chart files to Bridge Command world format

#include "WorldGenerator.hpp"

#include <iostream>
#include <string>
#include <sys/stat.h>

static void printUsage(const char* progName) {
    std::cerr << "Usage: " << progName << " --input <chart.000> --output <world_dir/> [options]\n"
              << "\n"
              << "Converts an S-57 chart file into a Bridge Command world directory.\n"
              << "\n"
              << "Options:\n"
              << "  --input <path>       Path to S-57 chart file (.000)\n"
              << "  --output <path>      Output world directory (created if needed)\n"
              << "  --resolution <N>     Heightmap resolution (default: 1025)\n"
              << "  --dem-dir <path>     Directory with Copernicus DEM .tif tiles for land elevation\n"
              << "                       (download with tools/download_dem.py)\n"
              << "  --bathy-dir <path>   Directory with BlueTopo .tif tiles for high-res bathymetry\n"
              << "  --help               Show this help\n";
}

static bool createDirectory(const std::string& path) {
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

int main(int argc, char* argv[]) {
    std::string inputPath;
    std::string outputDir;
    std::string demDir;
    std::string bathyDir;
    int resolution = 1025;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--input" && i + 1 < argc) {
            inputPath = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            outputDir = argv[++i];
        } else if (arg == "--dem-dir" && i + 1 < argc) {
            demDir = argv[++i];
        } else if (arg == "--bathy-dir" && i + 1 < argc) {
            bathyDir = argv[++i];
        } else if (arg == "--resolution" && i + 1 < argc) {
            resolution = std::atoi(argv[++i]);
            if (resolution < 33 || resolution > 4097) {
                std::cerr << "Error: Resolution must be between 33 and 4097\n";
                return 1;
            }
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (inputPath.empty() || outputDir.empty()) {
        std::cerr << "Error: --input and --output are required\n\n";
        printUsage(argv[0]);
        return 1;
    }

    // Create output directory
    if (!createDirectory(outputDir)) {
        std::cerr << "Error: Failed to create output directory: " << outputDir << "\n";
        return 1;
    }

    std::cout << "Converting chart: " << inputPath << "\n";
    std::cout << "Output directory: " << outputDir << "\n";
    std::cout << "Heightmap resolution: " << resolution << "x" << resolution << "\n";
    if (!demDir.empty()) {
        std::cout << "DEM tile directory: " << demDir << "\n";
    }
    if (!bathyDir.empty()) {
        std::cout << "Bathymetry tile directory: " << bathyDir << "\n";
    }
    std::cout << "\n";

    auto result = WorldGenerator::generateWorld(inputPath, outputDir, resolution, demDir, bathyDir);

    if (!result.success) {
        std::cerr << "Error: World generation failed\n";
        return 1;
    }

    std::cout << "Generated world with:\n";
    std::cout << "  " << result.buoyCount << " buoys\n";
    std::cout << "  " << result.lightCount << " lights\n";
    std::cout << "  " << result.landmarkCount << " landmarks\n";
    std::cout << "  " << result.depthAreaCount << " depth areas\n";
    std::cout << "  " << result.soundingCount << " soundings\n";
    std::cout << "  " << result.coastlineCount << " coastline segments\n";
    std::cout << "  Max height: " << result.maxHeight << " m\n";
    std::cout << "  Max depth: " << result.maxDepth << " m\n";
    std::cout << "\nFiles written to " << outputDir << "/\n";

    return 0;
}
