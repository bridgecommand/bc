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

#include "ChartReader.hpp"

#include "ogrsf_frmts.h"
#include "gdal_priv.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <iostream>

ChartReader::ChartReader() : dataset(nullptr), gdalInitialized(false) {
}

ChartReader::~ChartReader() {
    close();
}

bool ChartReader::open(const std::string& chartPath) {
    if (!gdalInitialized) {
        GDALAllRegister();
        gdalInitialized = true;
    }

    close(); // Close any previously opened dataset

    // S-57 open options for better handling of multipoint soundings
    const char* openOptions[] = {
        "SPLIT_MULTIPOINT=ON",
        "ADD_SOUNDG_DEPTH=ON",
        nullptr
    };

    dataset = static_cast<GDALDataset*>(
        GDALOpenEx(chartPath.c_str(), GDAL_OF_VECTOR | GDAL_OF_READONLY,
                   nullptr, openOptions, nullptr));

    if (!dataset) {
        std::cerr << "ChartReader: Failed to open " << chartPath << std::endl;
        return false;
    }

    return true;
}

void ChartReader::close() {
    if (dataset) {
        GDALClose(dataset);
        dataset = nullptr;
    }
}

std::vector<ChartBuoy> ChartReader::extractBuoys() {
    std::vector<ChartBuoy> buoys;

    if (!dataset) return buoys;

    // S-57 buoy layer names
    const char* buoyLayers[] = {
        "BOYLAT",  // Lateral buoys (port/starboard)
        "BOYCAR",  // Cardinal buoys (N/E/S/W)
        "BOYISD",  // Isolated danger buoys
        "BOYSAW",  // Safe water buoys
        "BOYSPP",  // Special purpose buoys
        nullptr
    };

    for (int i = 0; buoyLayers[i] != nullptr; i++) {
        OGRLayer* layer = dataset->GetLayerByName(buoyLayers[i]);
        if (!layer) continue;

        layer->ResetReading();
        OGRFeature* feature;
        while ((feature = layer->GetNextFeature()) != nullptr) {
            OGRGeometry* geom = feature->GetGeometryRef();
            if (!geom || geom->getGeometryType() != wkbPoint) {
                OGRFeature::DestroyFeature(feature);
                continue;
            }

            OGRPoint* point = static_cast<OGRPoint*>(geom);

            ChartBuoy buoy;
            buoy.longitude = point->getX();
            buoy.latitude = point->getY();
            buoy.layerName = buoyLayers[i];

            // Read attributes (check field exists before reading)
            int idx;
            idx = feature->GetFieldIndex("BOYSHP");
            buoy.shape = (idx >= 0) ? feature->GetFieldAsInteger(idx) : 0;

            idx = feature->GetFieldIndex("COLOUR");
            buoy.colours = (idx >= 0) ? feature->GetFieldAsString(idx) : "";

            idx = feature->GetFieldIndex("CATLAM");
            buoy.categoryLateral = (idx >= 0) ? feature->GetFieldAsInteger(idx) : 0;

            idx = feature->GetFieldIndex("CATCAM");
            buoy.categoryCardinal = (idx >= 0) ? feature->GetFieldAsInteger(idx) : 0;

            idx = feature->GetFieldIndex("CATSPM");
            buoy.categorySpecial = (idx >= 0) ? feature->GetFieldAsInteger(idx) : 0;

            idx = feature->GetFieldIndex("OBJNAM");
            buoy.name = (idx >= 0) ? feature->GetFieldAsString(idx) : "";

            buoys.push_back(buoy);
            OGRFeature::DestroyFeature(feature);
        }
    }

    return buoys;
}

std::vector<ChartLight> ChartReader::extractLights() {
    std::vector<ChartLight> lights;

    if (!dataset) return lights;

    OGRLayer* layer = dataset->GetLayerByName("LIGHTS");
    if (!layer) return lights;

    layer->ResetReading();
    OGRFeature* feature;
    while ((feature = layer->GetNextFeature()) != nullptr) {
        OGRGeometry* geom = feature->GetGeometryRef();
        if (!geom || geom->getGeometryType() != wkbPoint) {
            OGRFeature::DestroyFeature(feature);
            continue;
        }

        OGRPoint* point = static_cast<OGRPoint*>(geom);

        ChartLight light;
        light.longitude = point->getX();
        light.latitude = point->getY();

        int idx;
        idx = feature->GetFieldIndex("LITCHR");
        light.characteristic = (idx >= 0) ? feature->GetFieldAsInteger(idx) : 0;

        idx = feature->GetFieldIndex("SIGPER");
        light.period = (idx >= 0) ? feature->GetFieldAsDouble(idx) : 0.0;

        idx = feature->GetFieldIndex("SIGGRP");
        light.group = (idx >= 0) ? feature->GetFieldAsString(idx) : "";

        idx = feature->GetFieldIndex("SECTR1");
        light.sectorStart = (idx >= 0) ? feature->GetFieldAsDouble(idx) : 0.0;

        idx = feature->GetFieldIndex("SECTR2");
        light.sectorEnd = (idx >= 0) ? feature->GetFieldAsDouble(idx) : 360.0;

        idx = feature->GetFieldIndex("COLOUR");
        light.colour = (idx >= 0) ? feature->GetFieldAsInteger(idx) : 1; // Default white

        idx = feature->GetFieldIndex("VALNMR");
        light.range = (idx >= 0) ? feature->GetFieldAsDouble(idx) : 5.0; // Default 5 NM

        idx = feature->GetFieldIndex("HEIGHT");
        light.height = (idx >= 0) ? feature->GetFieldAsDouble(idx) : 5.0; // Default 5m

        light.buoyIndex = -1; // Will be resolved later

        lights.push_back(light);
        OGRFeature::DestroyFeature(feature);
    }

    return lights;
}

std::vector<DepthArea> ChartReader::extractDepthAreas() {
    std::vector<DepthArea> areas;

    if (!dataset) return areas;

    OGRLayer* layer = dataset->GetLayerByName("DEPARE");
    if (!layer) return areas;

    layer->ResetReading();
    OGRFeature* feature;
    while ((feature = layer->GetNextFeature()) != nullptr) {
        OGRGeometry* geom = feature->GetGeometryRef();
        if (!geom) {
            OGRFeature::DestroyFeature(feature);
            continue;
        }

        // DEPARE can be Polygon or MultiPolygon
        OGRwkbGeometryType gtype = wkbFlatten(geom->getGeometryType());
        if (gtype != wkbPolygon && gtype != wkbMultiPolygon) {
            OGRFeature::DestroyFeature(feature);
            continue;
        }

        DepthArea area;
        int idx;
        idx = feature->GetFieldIndex("DRVAL1");
        area.minDepth = (idx >= 0) ? feature->GetFieldAsDouble(idx) : 0.0;

        idx = feature->GetFieldIndex("DRVAL2");
        area.maxDepth = (idx >= 0) ? feature->GetFieldAsDouble(idx) : 0.0;

        // Extract outer ring points from the polygon(s)
        auto extractRing = [](const OGRPolygon* poly, std::vector<ChartPoint>& pts) {
            const OGRLinearRing* ring = poly->getExteriorRing();
            if (!ring) return;
            for (int i = 0; i < ring->getNumPoints(); i++) {
                pts.push_back({ring->getX(i), ring->getY(i)});
            }
        };

        if (gtype == wkbPolygon) {
            extractRing(static_cast<const OGRPolygon*>(geom), area.boundary);
            areas.push_back(area);
        } else {
            // MultiPolygon: emit one DepthArea per sub-polygon
            const OGRMultiPolygon* mp = static_cast<const OGRMultiPolygon*>(geom);
            for (int i = 0; i < mp->getNumGeometries(); i++) {
                DepthArea subArea;
                subArea.minDepth = area.minDepth;
                subArea.maxDepth = area.maxDepth;
                extractRing(static_cast<const OGRPolygon*>(mp->getGeometryRef(i)),
                            subArea.boundary);
                areas.push_back(subArea);
            }
        }

        OGRFeature::DestroyFeature(feature);
    }

    return areas;
}

std::vector<Sounding> ChartReader::extractSoundings() {
    std::vector<Sounding> soundings;

    if (!dataset) return soundings;

    OGRLayer* layer = dataset->GetLayerByName("SOUNDG");
    if (!layer) return soundings;

    layer->ResetReading();
    OGRFeature* feature;
    while ((feature = layer->GetNextFeature()) != nullptr) {
        OGRGeometry* geom = feature->GetGeometryRef();
        if (!geom) {
            OGRFeature::DestroyFeature(feature);
            continue;
        }

        // With SPLIT_MULTIPOINT=ON, soundings are individual points
        // With ADD_SOUNDG_DEPTH=ON, the Z value is also in a DEPTH field
        OGRwkbGeometryType gtype = wkbFlatten(geom->getGeometryType());

        if (gtype == wkbPoint) {
            OGRPoint* pt = static_cast<OGRPoint*>(geom);
            Sounding s;
            s.longitude = pt->getX();
            s.latitude = pt->getY();

            // Prefer DEPTH attribute (added by ADD_SOUNDG_DEPTH=ON)
            int idx = feature->GetFieldIndex("DEPTH");
            if (idx >= 0) {
                s.depth = feature->GetFieldAsDouble(idx);
            } else {
                // Fall back to Z coordinate
                s.depth = pt->getZ();
            }

            soundings.push_back(s);
        } else if (gtype == wkbMultiPoint) {
            // Fallback if SPLIT_MULTIPOINT wasn't applied
            const OGRMultiPoint* mp = static_cast<const OGRMultiPoint*>(geom);
            for (int i = 0; i < mp->getNumGeometries(); i++) {
                const OGRPoint* pt = static_cast<const OGRPoint*>(mp->getGeometryRef(i));
                Sounding s;
                s.longitude = pt->getX();
                s.latitude = pt->getY();
                s.depth = pt->getZ();
                soundings.push_back(s);
            }
        }

        OGRFeature::DestroyFeature(feature);
    }

    return soundings;
}

std::vector<CoastlineSegment> ChartReader::extractCoastlines() {
    std::vector<CoastlineSegment> coastlines;

    if (!dataset) return coastlines;

    // Try both COALNE (coastline) and LNDARE (land area) layers
    const char* coastLayers[] = {"COALNE", "LNDARE", nullptr};

    for (int li = 0; coastLayers[li] != nullptr; li++) {
        OGRLayer* layer = dataset->GetLayerByName(coastLayers[li]);
        if (!layer) continue;

        layer->ResetReading();
        OGRFeature* feature;
        while ((feature = layer->GetNextFeature()) != nullptr) {
            OGRGeometry* geom = feature->GetGeometryRef();
            if (!geom) {
                OGRFeature::DestroyFeature(feature);
                continue;
            }

            OGRwkbGeometryType gtype = wkbFlatten(geom->getGeometryType());

            auto extractLineString = [](const OGRLineString* ls, CoastlineSegment& seg) {
                for (int i = 0; i < ls->getNumPoints(); i++) {
                    seg.points.push_back({ls->getX(i), ls->getY(i)});
                }
            };

            if (gtype == wkbLineString) {
                CoastlineSegment seg;
                extractLineString(static_cast<const OGRLineString*>(geom), seg);
                if (!seg.points.empty()) coastlines.push_back(seg);
            } else if (gtype == wkbMultiLineString) {
                const OGRMultiLineString* mls = static_cast<const OGRMultiLineString*>(geom);
                for (int i = 0; i < mls->getNumGeometries(); i++) {
                    CoastlineSegment seg;
                    extractLineString(
                        static_cast<const OGRLineString*>(mls->getGeometryRef(i)), seg);
                    if (!seg.points.empty()) coastlines.push_back(seg);
                }
            } else if (gtype == wkbPolygon) {
                // LNDARE uses polygons - extract outer ring as coastline
                const OGRPolygon* poly = static_cast<const OGRPolygon*>(geom);
                const OGRLinearRing* ring = poly->getExteriorRing();
                if (ring) {
                    CoastlineSegment seg;
                    for (int i = 0; i < ring->getNumPoints(); i++) {
                        seg.points.push_back({ring->getX(i), ring->getY(i)});
                    }
                    if (!seg.points.empty()) coastlines.push_back(seg);
                }
            } else if (gtype == wkbMultiPolygon) {
                const OGRMultiPolygon* mp = static_cast<const OGRMultiPolygon*>(geom);
                for (int i = 0; i < mp->getNumGeometries(); i++) {
                    const OGRPolygon* poly =
                        static_cast<const OGRPolygon*>(mp->getGeometryRef(i));
                    const OGRLinearRing* ring = poly->getExteriorRing();
                    if (ring) {
                        CoastlineSegment seg;
                        for (int j = 0; j < ring->getNumPoints(); j++) {
                            seg.points.push_back({ring->getX(j), ring->getY(j)});
                        }
                        if (!seg.points.empty()) coastlines.push_back(seg);
                    }
                }
            }

            OGRFeature::DestroyFeature(feature);
        }
    }

    return coastlines;
}

std::vector<ChartLandmark> ChartReader::extractLandmarks() {
    std::vector<ChartLandmark> landmarks;

    if (!dataset) return landmarks;

    // LNDMRK = landmarks (towers, chimneys, monuments, etc.)
    // BUISGL = single buildings
    const char* landmarkLayers[] = {"LNDMRK", "BUISGL", nullptr};

    for (int li = 0; landmarkLayers[li] != nullptr; li++) {
        OGRLayer* layer = dataset->GetLayerByName(landmarkLayers[li]);
        if (!layer) continue;

        layer->ResetReading();
        OGRFeature* feature;
        while ((feature = layer->GetNextFeature()) != nullptr) {
            OGRGeometry* geom = feature->GetGeometryRef();
            if (!geom) {
                OGRFeature::DestroyFeature(feature);
                continue;
            }

            // Landmarks are typically points; buildings may be polygons (use centroid)
            double lon = 0, lat = 0;
            OGRwkbGeometryType gtype = wkbFlatten(geom->getGeometryType());
            if (gtype == wkbPoint) {
                OGRPoint* pt = static_cast<OGRPoint*>(geom);
                lon = pt->getX();
                lat = pt->getY();
            } else {
                // For polygon buildings, use centroid
                OGRPoint centroid;
                if (geom->Centroid(&centroid) == OGRERR_NONE) {
                    lon = centroid.getX();
                    lat = centroid.getY();
                } else {
                    OGRFeature::DestroyFeature(feature);
                    continue;
                }
            }

            ChartLandmark lm;
            lm.longitude = lon;
            lm.latitude = lat;
            lm.layerName = landmarkLayers[li];

            int idx;
            idx = feature->GetFieldIndex("CATLMK");
            lm.category = (idx >= 0) ? feature->GetFieldAsInteger(idx) : 0;

            idx = feature->GetFieldIndex("HEIGHT");
            lm.height = (idx >= 0) ? feature->GetFieldAsDouble(idx) : 0.0;

            idx = feature->GetFieldIndex("OBJNAM");
            lm.name = (idx >= 0) ? feature->GetFieldAsString(idx) : "";

            landmarks.push_back(lm);
            OGRFeature::DestroyFeature(feature);
        }
    }

    return landmarks;
}

std::vector<UrbanArea> ChartReader::extractUrbanAreas() {
    std::vector<UrbanArea> areas;
    if (!dataset) return areas;

    // BUAARE = Built-up Area in S-57
    const char* layerName = "BUAARE";
    OGRLayer* layer = dataset->GetLayerByName(layerName);
    if (!layer) return areas;

    layer->ResetReading();
    OGRFeature* feature = nullptr;
    while ((feature = layer->GetNextFeature()) != nullptr) {
        OGRGeometry* geom = feature->GetGeometryRef();
        if (!geom) {
            OGRFeature::DestroyFeature(feature);
            continue;
        }

        auto extractRing = [](const OGRLinearRing* ring, UrbanArea& ua) {
            int nPoints = ring->getNumPoints();
            for (int i = 0; i < nPoints; i++) {
                ChartPoint pt;
                pt.longitude = ring->getX(i);
                pt.latitude = ring->getY(i);
                ua.boundary.push_back(pt);
            }
        };

        OGRwkbGeometryType type = wkbFlatten(geom->getGeometryType());
        if (type == wkbPolygon) {
            OGRPolygon* poly = static_cast<OGRPolygon*>(geom);
            UrbanArea ua;
            extractRing(poly->getExteriorRing(), ua);
            if (!ua.boundary.empty()) areas.push_back(ua);
        } else if (type == wkbMultiPolygon) {
            OGRMultiPolygon* mp = static_cast<OGRMultiPolygon*>(geom);
            for (int i = 0; i < mp->getNumGeometries(); i++) {
                OGRPolygon* poly = static_cast<OGRPolygon*>(mp->getGeometryRef(i));
                UrbanArea ua;
                extractRing(poly->getExteriorRing(), ua);
                if (!ua.boundary.empty()) areas.push_back(ua);
            }
        }

        OGRFeature::DestroyFeature(feature);
    }

    return areas;
}

// ── Buoy type mapping ──────────────────────────────────────────────────────

std::string ChartReader::mapBuoyType(const ChartBuoy& buoy) {
    // Lateral buoys
    if (buoy.layerName == "BOYLAT") {
        if (buoy.categoryLateral == 1) {
            // Port (IALA A: red, IALA B: green - we assume IALA B for US charts)
            // In IALA B (Americas), CATLAM=1 (port hand) marks are red
            if (buoy.shape == 5) return "port_post";  // spar/post
            if (buoy.shape == 4) return "port_post";  // pillar
            return "port_med";
        }
        if (buoy.categoryLateral == 2) {
            // Starboard
            if (buoy.shape == 5) return "stbd_post";
            if (buoy.shape == 4) return "stbd_post";
            return "stbd_med";
        }
        if (buoy.categoryLateral == 3) {
            // Preferred channel to starboard
            return "pref_stbd_small";
        }
        if (buoy.categoryLateral == 4) {
            // Preferred channel to port
            return "pref_port_small";
        }
    }

    // Cardinal buoys
    if (buoy.layerName == "BOYCAR") {
        if (buoy.categoryCardinal == 1) return "north_small";
        if (buoy.categoryCardinal == 2) return "east_small";
        if (buoy.categoryCardinal == 3) return "south_small";
        if (buoy.categoryCardinal == 4) return "west_small";
    }

    // Isolated danger
    if (buoy.layerName == "BOYISD") {
        return "black"; // Black with red band
    }

    // Safe water
    if (buoy.layerName == "BOYSAW") {
        return "safe";
    }

    // Special purpose
    if (buoy.layerName == "BOYSPP") {
        if (buoy.shape == 5 || buoy.shape == 4) return "special_post";
        return "special_1";
    }

    // Default fallback
    return "port_small";
}

// ── Landmark type mapping ─────────────────────────────────────────────────

std::string ChartReader::mapLandmarkType(const ChartLandmark& landmark) {
    // BUISGL (buildings) default to House
    if (landmark.layerName == "BUISGL") {
        return "House";
    }

    // Map S-57 CATLMK codes to available BC LandObject model names
    switch (landmark.category) {
        case 1:  return "Beacon";     // cairn
        case 3:  return "Chimneys";   // chimney
        case 5:  return "Flagstaff";  // flagstaff/flagpole
        case 7:  return "Masts";      // mast
        case 9:  return "Cross";      // monument
        case 14: return "Cross";      // cross
        case 15: return "Church";     // dome
        case 17: return "Tower";      // tower
        case 18: return "Masts";      // windmill (no specific model)
        case 19: return "Masts";      // windmotor (no specific model)
        case 20: return "Church";     // spire/minaret
        default: return "House";      // unknown landmark type
    }
}

// ── Land object INI generation ────────────────────────────────────────────

std::string ChartReader::generateLandObjectIni(const std::vector<ChartLandmark>& landmarks) {
    std::ostringstream oss;
    oss << "Number=" << landmarks.size() << "\n\n";

    for (size_t i = 0; i < landmarks.size(); i++) {
        int idx = static_cast<int>(i) + 1; // 1-indexed
        std::string type = mapLandmarkType(landmarks[i]);

        oss << "Type(" << idx << ")=" << type << "\n";
        oss.precision(7);
        oss << std::fixed;
        oss << "Long(" << idx << ")=" << landmarks[i].longitude << "\n";
        oss << "Lat(" << idx << ")=" << landmarks[i].latitude << "\n";

        // Height above ground (0 means use model default)
        if (landmarks[i].height > 0) {
            oss.precision(1);
            oss << "HeightAbove(" << idx << ")=" << landmarks[i].height << "\n";
        }

        oss << "Rotation(" << idx << ")=0\n";
        oss << "\n";
    }

    return oss.str();
}

// ── Light sequence generation ──────────────────────────────────────────────

std::string ChartReader::lightCharacteristicToSequence(int litchr, double period,
                                                        const std::string& group) {
    // Each character in sequence represents 0.25 seconds
    // L = light on, D = dark
    // Total sequence length = period * 4 (quarter-second resolution)

    if (period <= 0) period = 4.0; // Default 4 second period

    int totalSlots = static_cast<int>(period * 4);
    if (totalSlots < 4) totalSlots = 4;
    if (totalSlots > 100) totalSlots = 100; // Safety cap

    std::string seq;

    // Parse group number if present, e.g. "(2)" -> 2 flashes
    int groupCount = 1;
    if (!group.empty()) {
        for (char c : group) {
            if (c >= '1' && c <= '9') {
                groupCount = c - '0';
                break;
            }
        }
    }

    switch (litchr) {
        case 1: // Fixed (F) - always on
            seq = std::string(totalSlots, 'L');
            break;

        case 2: // Flashing (Fl)
            if (groupCount == 1) {
                // Single flash: 1s light, rest dark
                int lightSlots = 4; // 1 second
                seq = std::string(lightSlots, 'L');
                seq += std::string(totalSlots - lightSlots, 'D');
            } else {
                // Group flash: multiple short flashes
                int flashLen = 2;  // 0.5s each flash
                int gapLen = 2;    // 0.5s gap between flashes
                for (int i = 0; i < groupCount; i++) {
                    seq += std::string(flashLen, 'L');
                    if (i < groupCount - 1) {
                        seq += std::string(gapLen, 'D');
                    }
                }
                // Fill remaining with dark
                if (static_cast<int>(seq.length()) < totalSlots) {
                    seq += std::string(totalSlots - seq.length(), 'D');
                }
            }
            break;

        case 3: // Long Flash (LFl)
            {
                int lightSlots = 8; // 2 seconds
                seq = std::string(lightSlots, 'L');
                seq += std::string(totalSlots - lightSlots, 'D');
            }
            break;

        case 4: // Quick (Q)
            // 0.25s on, 0.75s off repeated
            for (int i = 0; i < totalSlots; i++) {
                seq += (i % 4 == 0) ? 'L' : 'D';
            }
            break;

        case 5: // Very Quick (VQ)
            // 0.25s on, 0.25s off repeated
            for (int i = 0; i < totalSlots; i++) {
                seq += (i % 2 == 0) ? 'L' : 'D';
            }
            break;

        case 7: // Isophase (Iso) - equal light and dark
            {
                int halfSlots = totalSlots / 2;
                seq = std::string(halfSlots, 'L');
                seq += std::string(totalSlots - halfSlots, 'D');
            }
            break;

        case 8: // Occulting (Oc) - mostly on with short dark period
            {
                int darkSlots = 4; // 1 second dark
                int lightSlots = totalSlots - darkSlots;
                seq = std::string(lightSlots, 'L');
                seq += std::string(darkSlots, 'D');
            }
            break;

        default:
            // Unknown: treat as flashing with 4s period
            seq = std::string(4, 'L');
            seq += std::string(totalSlots - 4, 'D');
            break;
    }

    return seq;
}

// ── Colour mapping ─────────────────────────────────────────────────────────

void ChartReader::colourToRGB(int colourCode, int& r, int& g, int& b) {
    switch (colourCode) {
        case 1:  r = 255; g = 255; b = 255; break; // White
        case 3:  r = 255; g = 0;   b = 0;   break; // Red
        case 4:  r = 0;   g = 255; b = 0;   break; // Green
        case 5:  r = 0;   g = 0;   b = 255; break; // Blue
        case 6:  r = 255; g = 255; b = 0;   break; // Yellow
        case 11: r = 255; g = 165; b = 0;   break; // Orange
        default: r = 255; g = 255; b = 255; break; // Default white
    }
}

// ── Proximity matching ─────────────────────────────────────────────────────

int ChartReader::findClosestBuoy(double lon, double lat,
                                  const std::vector<ChartBuoy>& buoys,
                                  double maxDistMetres) {
    int closestIdx = -1;
    double closestDist = maxDistMetres;

    for (size_t i = 0; i < buoys.size(); i++) {
        // Approximate distance in metres using lat/lon
        double dLat = (lat - buoys[i].latitude) * 111320.0;
        double dLon = (lon - buoys[i].longitude) * 111320.0 * cos(lat * M_PI / 180.0);
        double dist = sqrt(dLat * dLat + dLon * dLon);

        if (dist < closestDist) {
            closestDist = dist;
            closestIdx = static_cast<int>(i);
        }
    }

    return closestIdx;
}

// ── INI generation ─────────────────────────────────────────────────────────

std::string ChartReader::generateBuoyIni(const std::vector<ChartBuoy>& buoys) {
    std::ostringstream oss;
    oss << "Number=" << buoys.size() << "\n\n";

    for (size_t i = 0; i < buoys.size(); i++) {
        int idx = static_cast<int>(i) + 1; // 1-indexed
        std::string type = mapBuoyType(buoys[i]);

        oss << "Type(" << idx << ")=" << type << "\n";
        oss.precision(7);
        oss << std::fixed;
        oss << "Long(" << idx << ")=" << buoys[i].longitude << "\n";
        oss << "Lat(" << idx << ")=" << buoys[i].latitude << "\n";

        // Posts and pillars are grounded (not floating)
        if (buoys[i].shape == 4 || buoys[i].shape == 5) {
            oss << "Grounded(" << idx << ")=1\n";
        }

        oss << "\n";
    }

    return oss.str();
}

std::string ChartReader::generateLightIni(const std::vector<ChartBuoy>& buoys,
                                           const std::vector<ChartLight>& lights) {
    // First, resolve which lights are on buoys
    std::vector<ChartLight> resolvedLights = lights;
    for (auto& light : resolvedLights) {
        light.buoyIndex = findClosestBuoy(light.longitude, light.latitude, buoys);
    }

    // Only include lights that are on buoys (matching existing BC behavior)
    // or standalone lights with known positions
    std::vector<size_t> includedIndices;
    for (size_t i = 0; i < resolvedLights.size(); i++) {
        if (resolvedLights[i].buoyIndex >= 0) {
            includedIndices.push_back(i);
        }
    }

    std::ostringstream oss;
    oss << "Number=" << includedIndices.size() << "\n\n";

    for (size_t n = 0; n < includedIndices.size(); n++) {
        const ChartLight& light = resolvedLights[includedIndices[n]];
        int idx = static_cast<int>(n) + 1; // 1-indexed

        std::string seq = lightCharacteristicToSequence(
            light.characteristic, light.period, light.group);

        int r, g, b;
        colourToRGB(light.colour, r, g, b);

        // Buoy reference is 1-indexed in BC format
        int buoyRef = light.buoyIndex + 1;

        oss << "Buoy(" << idx << ")=" << buoyRef << "\n";
        oss << "Sequence(" << idx << ")=" << seq << "\n";
        oss.precision(1);
        oss << std::fixed;
        oss << "Range(" << idx << ")=" << light.range << "\n";
        oss << "PhaseStart(" << idx << ")=" << idx << "\n"; // Stagger phases
        oss << "Height(" << idx << ")=" << light.height << "\n";
        oss << "Absolute(" << idx << ")=2\n"; // Relative to buoy
        oss << "Red(" << idx << ")=" << r << "\n";
        oss << "Green(" << idx << ")=" << g << "\n";
        oss << "Blue(" << idx << ")=" << b << "\n";

        // Sector angles
        if (light.sectorStart == 0.0 && light.sectorEnd >= 360.0) {
            // All-round light
            oss << "StartAngle(" << idx << ")=0\n";
            oss << "EndAngle(" << idx << ")=360\n";
        } else {
            oss << "StartAngle(" << idx << ")=" << light.sectorStart << "\n";
            oss << "EndAngle(" << idx << ")=" << light.sectorEnd << "\n";
        }

        oss << "\n";
    }

    return oss.str();
}

#endif // WITH_GDAL
