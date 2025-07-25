// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

// The code for the TerrainTriangleSelector is based on the GeoMipMapSelector
// developed by Spintz. He made it available for Irrlicht and allowed it to be
// distributed under this licence. I only modified some parts. A lot of thanks go to him.

#ifndef __BC_TERRAIN_TRIANGLE_SELECTOR_H__
#define __BC_TERRAIN_TRIANGLE_SELECTOR_H__

#include "ITriangleSelector.h"
#include "irrArray.h"

namespace irr
{
namespace scene
{

class BCTerrainSceneNode;

//! Triangle Selector for the TerrainSceneNode
/** The code for the TerrainTriangleSelector is based on the GeoMipMapSelector
developed by Spintz. He made it available for Irrlicht and allowed it to be
distributed under this license. I only modified some parts. A lot of thanks go
to him.
*/
class BCTerrainTriangleSelector : public ITriangleSelector
{
public:

	//! Constructs a selector based on an ITerrainSceneNode
	BCTerrainTriangleSelector(BCTerrainSceneNode* node, s32 LOD);

	//! Destructor
	virtual ~BCTerrainTriangleSelector();

	//! Clears and sets triangle data
	virtual void setTriangleData(BCTerrainSceneNode* node, s32 LOD);

	//! Gets all triangles.
	void getTriangles(core::triangle3df* triangles, s32 arraySize, s32& outTriangleCount,
		const core::matrix4* transform, bool useNodeTransform,
		irr::core::array<SCollisionTriangleRange>* outTriangleInfo) const IRR_OVERRIDE;

	//! Gets all triangles which lie within a specific bounding box.
	void getTriangles(core::triangle3df* triangles, s32 arraySize, s32& outTriangleCount,
		const core::aabbox3d<f32>& box, const core::matrix4* transform, bool useNodeTransform,
		irr::core::array<SCollisionTriangleRange>* outTriangleInfo) const IRR_OVERRIDE;

	//! Gets all triangles which have or may have contact with a 3d line.
	virtual void getTriangles(core::triangle3df* triangles, s32 arraySize,
		s32& outTriangleCount, const core::line3d<f32>& line,
		const core::matrix4* transform, bool useNodeTransform,
		irr::core::array<SCollisionTriangleRange>* outTriangleInfo) const IRR_OVERRIDE;

	//! Returns amount of all available triangles in this selector
	virtual s32 getTriangleCount() const IRR_OVERRIDE;

	//! Return the scene node associated with a given triangle.
	virtual ISceneNode* getSceneNodeForTriangle(u32 triangleIndex) const IRR_OVERRIDE;

	// Get the number of TriangleSelectors that are part of this one
	virtual u32 getSelectorCount() const IRR_OVERRIDE;

	// Get the TriangleSelector based on index based on getSelectorCount
	virtual ITriangleSelector* getSelector(u32 index) IRR_OVERRIDE;

	// Get the TriangleSelector based on index based on getSelectorCount
	virtual const ITriangleSelector* getSelector(u32 index) const IRR_OVERRIDE;

private:

	friend class BCTerrainSceneNode;

	struct SGeoMipMapTrianglePatch
	{
		core::array<core::triangle3df> Triangles;
		s32 NumTriangles;
		core::aabbox3df Box;
	};

	struct SGeoMipMapTrianglePatches
	{
		SGeoMipMapTrianglePatches() :
			NumPatches(0), TotalTriangles(0)
		{
		}

		core::array<SGeoMipMapTrianglePatch> TrianglePatchArray;
		s32 NumPatches;
		u32 TotalTriangles;
	};

	BCTerrainSceneNode* SceneNode;
	SGeoMipMapTrianglePatches TrianglePatches;
};

} // end namespace scene
} // end namespace irr


#endif // __BC_TERRAIN_TRIANGLE_SELECTOR_H__
