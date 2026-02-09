#!/usr/bin/env python3
"""
Download Copernicus DEM GLO-30 tiles from AWS S3 for a given bounding box.

Copernicus DEM is freely available on s3://copernicus-dem-30m/ (no auth required).
Each tile covers 1 degree x 1 degree at ~30m resolution.

Usage:
    python3 download_dem.py <south_lat> <north_lat> <west_lon> <east_lon> <output_dir>

Example:
    python3 download_dem.py 48.4 48.6 -122.6 -122.4 ./dem_tiles/

This will download all DEM tiles that cover the requested bounding box.
Requires: AWS CLI installed (aws s3 cp --no-sign-request, no credentials needed)
"""

import os
import sys
import subprocess
import math


S3_BUCKET = "s3://copernicus-dem-30m"


def tile_name(lat_int: int, lon_int: int) -> str:
    """Generate Copernicus DEM tile name for a given integer lat/lon (SW corner).

    Tile naming convention:
        Copernicus_DSM_COG_10_N48_00_W123_00_DEM
    Where:
        N/S = hemisphere for latitude
        E/W = hemisphere for longitude
        Latitude is 2 digits, longitude is 3 digits
    """
    lat_prefix = "N" if lat_int >= 0 else "S"
    lon_prefix = "E" if lon_int >= 0 else "W"

    lat_abs = abs(lat_int)
    lon_abs = abs(lon_int)

    return f"Copernicus_DSM_COG_10_{lat_prefix}{lat_abs:02d}_00_{lon_prefix}{lon_abs:03d}_00_DEM"


def compute_tiles(south_lat: float, north_lat: float,
                  west_lon: float, east_lon: float) -> list:
    """Compute list of (lat_int, lon_int) pairs for tiles covering the bbox.

    Each tile covers from its integer coordinate to +1 degree in each direction.
    For example, tile N48_W123 covers lat 48-49, lon -123 to -122.
    """
    # Each tile at (lat, lon) covers [lat, lat+1) x [lon, lon+1).
    # Floor the SW corner, and for the NE corner we need the tile containing
    # that point (which is floor of the coordinate, unless exactly on boundary).
    lat_start = math.floor(south_lat)
    lon_start = math.floor(west_lon)

    # For north/east edges: floor gives the right tile, but if exactly on a
    # tile boundary (e.g. 49.0), the previous tile (48) already covers it.
    lat_end = math.ceil(north_lat) if north_lat != math.floor(north_lat) else int(north_lat)
    lon_end = math.ceil(east_lon) if east_lon != math.floor(east_lon) else int(east_lon)

    tiles = []
    for lat in range(lat_start, lat_end):
        for lon in range(lon_start, lon_end):
            tiles.append((lat, lon))

    return tiles


def download_tile(lat_int: int, lon_int: int, output_dir: str) -> bool:
    """Download a single DEM tile from S3. Returns True on success."""
    name = tile_name(lat_int, lon_int)
    s3_path = f"{S3_BUCKET}/{name}/{name}.tif"
    local_path = os.path.join(output_dir, f"{name}.tif")

    if os.path.exists(local_path):
        size = os.path.getsize(local_path)
        if size > 0:
            print(f"  [skip] {name}.tif already exists ({size / 1024 / 1024:.1f} MB)")
            return True

    print(f"  [download] {name}.tif ...")
    cmd = ["aws", "s3", "cp", s3_path, local_path, "--no-sign-request"]

    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
        if result.returncode == 0:
            size = os.path.getsize(local_path)
            print(f"  [done] {name}.tif ({size / 1024 / 1024:.1f} MB)")
            return True
        else:
            # Tile might not exist (ocean areas often have no DEM coverage)
            stderr = result.stderr.strip()
            if "NoSuchKey" in stderr or "404" in stderr:
                print(f"  [missing] {name}.tif - no DEM coverage for this tile (ocean area?)")
            else:
                print(f"  [error] {name}.tif - {stderr}")
            return False
    except FileNotFoundError:
        print("ERROR: 'aws' CLI not found. Install AWS CLI: https://aws.amazon.com/cli/")
        print("       No AWS credentials are needed (uses --no-sign-request)")
        sys.exit(1)
    except subprocess.TimeoutExpired:
        print(f"  [timeout] {name}.tif - download timed out after 5 minutes")
        return False


def main():
    if len(sys.argv) != 6:
        print(__doc__)
        sys.exit(1)

    try:
        south_lat = float(sys.argv[1])
        north_lat = float(sys.argv[2])
        west_lon = float(sys.argv[3])
        east_lon = float(sys.argv[4])
    except ValueError:
        print("ERROR: All coordinates must be numbers")
        print(__doc__)
        sys.exit(1)

    output_dir = sys.argv[5]

    # Validate
    if south_lat >= north_lat:
        print(f"ERROR: south_lat ({south_lat}) must be less than north_lat ({north_lat})")
        sys.exit(1)
    if west_lon >= east_lon:
        print(f"ERROR: west_lon ({west_lon}) must be less than east_lon ({east_lon})")
        sys.exit(1)
    if south_lat < -90 or north_lat > 90:
        print("ERROR: Latitude must be between -90 and 90")
        sys.exit(1)
    if west_lon < -180 or east_lon > 180:
        print("ERROR: Longitude must be between -180 and 180")
        sys.exit(1)

    os.makedirs(output_dir, exist_ok=True)

    tiles = compute_tiles(south_lat, north_lat, west_lon, east_lon)

    print(f"Copernicus DEM GLO-30 Downloader")
    print(f"  Bounding box: ({south_lat}, {west_lon}) to ({north_lat}, {east_lon})")
    print(f"  Tiles needed: {len(tiles)}")
    print(f"  Output: {output_dir}")
    print()

    success_count = 0
    for lat_int, lon_int in tiles:
        if download_tile(lat_int, lon_int, output_dir):
            success_count += 1

    print()
    print(f"Downloaded {success_count}/{len(tiles)} tiles to {output_dir}")

    if success_count > 0:
        print(f"\nTo merge into Bridge Command world, use:")
        print(f"  bc-chart-converter --input chart.000 --output World/MyWorld/ --dem-dir {output_dir}")


if __name__ == "__main__":
    main()
