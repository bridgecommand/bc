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

#ifndef __CHARTREADER_HPP_INCLUDED__
#define __CHARTREADER_HPP_INCLUDED__

#ifdef WITH_GDAL

#include <string>
#include <vector>

// Forward declare GDAL types to avoid exposing GDAL headers to consumers
class GDALDataset;

struct ChartBuoy {
    double longitude;
    double latitude;
    int shape;           // BOYSHP: 1=conical, 2=can, 3=spherical, 4=pillar, 5=spar, 6=barrel, 7=super-buoy
    std::string colours; // COLOUR attribute (comma-separated IHO codes)
    int categoryLateral; // CATLAM: 1=port, 2=starboard, 3=pref_channel_stbd, 4=pref_channel_port
    int categoryCardinal;// CATCAM: 1=north, 2=east, 3=south, 4=west
    int categorySpecial; // CATSPM values
    std::string name;    // OBJNAM if present
    std::string layerName; // Which S-57 layer this came from (BOYLAT, BOYCAR, etc.)
};

struct ChartLight {
    double longitude;
    double latitude;
    int characteristic;  // LITCHR: 1=F, 2=Fl, 3=LFl, 4=Q, 5=VQ, 7=Iso, 8=Oc
    double period;       // SIGPER in seconds
    std::string group;   // SIGGRP e.g. "(2)"
    double sectorStart;  // SECTR1 in degrees (0 if all-round)
    double sectorEnd;    // SECTR2 in degrees (360 if all-round)
    int colour;          // COLOUR: 1=W, 3=R, 4=G, 5=Bu, 6=Y
    double range;        // VALNMR in NM
    double height;       // HEIGHT in metres
    int buoyIndex;       // -1 if not on a buoy, otherwise index into buoy list
};

class ChartReader {
public:
    ChartReader();
    ~ChartReader();

    bool open(const std::string& chartPath);
    void close();

    // Extract features from the opened chart
    std::vector<ChartBuoy> extractBuoys();
    std::vector<ChartLight> extractLights();

    // Map S-57 buoy data to Bridge Command buoy model name
    static std::string mapBuoyType(const ChartBuoy& buoy);

    // Generate Bridge Command ini file content
    static std::string generateBuoyIni(const std::vector<ChartBuoy>& buoys);
    static std::string generateLightIni(const std::vector<ChartBuoy>& buoys,
                                         const std::vector<ChartLight>& lights);

    // Convert S-57 light characteristic to BC Sequence string (L=light, D=dark)
    static std::string lightCharacteristicToSequence(int litchr, double period,
                                                      const std::string& group);

    // Convert S-57 colour code to RGB
    static void colourToRGB(int colourCode, int& r, int& g, int& b);

private:
    GDALDataset* dataset;
    bool gdalInitialized;

    // Helper: find closest buoy index for a light position
    static int findClosestBuoy(double lon, double lat,
                                const std::vector<ChartBuoy>& buoys,
                                double maxDistMetres = 50.0);
};

#endif // WITH_GDAL
#endif // __CHARTREADER_HPP_INCLUDED__
