#include "Collision.hpp"
#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"

void Collision::load(irr::scene::ISceneManager *aSmgr, irr::scene::IMeshSceneNode *aShipScene, irr::IrrlichtDevice *aDev, SimulationModel *aModel, float aHeightCorr)
{
  mBuoyCollision = false;
  mOtherShipCollision = false;

  // Detect sample points for terrain interaction here (think separately about how to do this for 360 models, probably with a separate collision model)
  // Add a triangle selector
  mShipScene = aShipScene;
  mDevice = aDev;
  mModel = aModel;
  mHeightCorr = aHeightCorr;
  mSmgr = aSmgr;
  
  mSelector = aSmgr->createTriangleSelector(mShipScene->getMesh(), mShipScene);
  if (mSelector)
    {
      mDevice->getLogger()->log("Created triangle selector");
      mShipScene->setTriangleSelector(mSelector);
    }
  mTriangleSelectorEnabled = true;

  mShipScene->updateAbsolutePosition();

  irr::core::aabbox3df boundingBox = mShipScene->getTransformedBoundingBox();
  irr::f32 minX = boundingBox.MinEdge.X;
  irr::f32 maxX = boundingBox.MaxEdge.X;
  irr::f32 minY = boundingBox.MinEdge.Y;
  irr::f32 maxY = boundingBox.MaxEdge.Y;
  irr::f32 minZ = boundingBox.MinEdge.Z;
  irr::f32 maxZ = boundingBox.MaxEdge.Z;

  // Find if we need more contact points to maintain minContactPointSpacing
  if (mModel->getModelParameters().minContactPointSpacing > 0)
    {
      mModel->getModelParameters().numberOfContactPoints.X = std::max(mModel->getModelParameters().numberOfContactPoints.X, (int)ceil((maxX - minX) / mModel->getModelParameters().minContactPointSpacing));
      mModel->getModelParameters().numberOfContactPoints.Y = std::max(mModel->getModelParameters().numberOfContactPoints.Y, (int)ceil((maxY - minY) / mModel->getModelParameters().minContactPointSpacing));
      mModel->getModelParameters().numberOfContactPoints.Z = std::max(mModel->getModelParameters().numberOfContactPoints.Z, (int)ceil((maxZ - minZ) / mModel->getModelParameters().minContactPointSpacing));
    }

  // Grid from below looking up
  for (int i = 0; i < mModel->getModelParameters().numberOfContactPoints.X; i++)
    {
      for (int j = 0; j < mModel->getModelParameters().numberOfContactPoints.Z; j++)
        {

	  irr::f32 xSpacing = (maxX - minX) / (irr::f32)(mModel->getModelParameters().numberOfContactPoints.X - 1);
	  irr::f32 zSpacing = (maxZ - minZ) / (irr::f32)(mModel->getModelParameters().numberOfContactPoints.Z - 1);

	  irr::f32 xTestPos = minX + (irr::f32)i * xSpacing;
	  irr::f32 zTestPos = minZ + (irr::f32)j * zSpacing;

	  irr::core::line3df ray; // Make a ray. This will start outside the mesh, looking in
	  ray.start.X = xTestPos;
	  ray.start.Y = minY - 0.1;
	  ray.start.Z = zTestPos;
	  ray.end = ray.start;
	  ray.end.Y = maxY + 0.1;

	  // Check the ray and add the contact point if it exists
	  addContactPointFromRay(ray, xSpacing * zSpacing);
        }
    }

  // Grid from ahead/astern
  for (int i = 0; i < mModel->getModelParameters().numberOfContactPoints.X; i++)
    {
      for (int j = 0; j < mModel->getModelParameters().numberOfContactPoints.Y; j++)
        {

	  irr::f32 xSpacing = (maxX - minX) / (irr::f32)(mModel->getModelParameters().numberOfContactPoints.X - 1);
	  irr::f32 ySpacing = (maxY - minY) / (irr::f32)(mModel->getModelParameters().numberOfContactPoints.Y - 1);

	  irr::f32 xTestPos = minX + (irr::f32)i * xSpacing;
	  irr::f32 yTestPos = minY + (irr::f32)j * ySpacing;

	  irr::core::line3df ray; // Make a ray. This will start outside the mesh, looking in
	  ray.start.X = xTestPos;
	  ray.start.Y = yTestPos;
	  ray.start.Z = maxZ + 0.1;
	  ray.end = ray.start;
	  ray.end.Z = minZ - 0.1;

	  // Check the ray and add the contact point if it exists
	  addContactPointFromRay(ray, xSpacing * ySpacing);
	  // swap ray direction and check again
	  ray.start.Z = minZ - 0.1;
	  ray.end.Z = maxZ + 0.1;
	  addContactPointFromRay(ray, xSpacing * ySpacing);
        }
    }

  // Grid from side to side
  for (int i = 0; i < mModel->getModelParameters().numberOfContactPoints.Z; i++)
    {
      for (int j = 0; j < mModel->getModelParameters().numberOfContactPoints.Y; j++)
        {

	  irr::f32 zSpacing = (maxZ - minZ) / (irr::f32)(mModel->getModelParameters().numberOfContactPoints.Z - 1);
	  irr::f32 ySpacing = (maxY - minY) / (irr::f32)(mModel->getModelParameters().numberOfContactPoints.Y - 1);

	  irr::f32 zTestPos = minZ + (irr::f32)i * zSpacing;
	  irr::f32 yTestPos = minY + (irr::f32)j * ySpacing;

	  irr::core::line3df ray; // Make a ray. This will start outside the mesh, looking in
	  ray.start.X = maxX + 0.1;
	  ray.start.Y = yTestPos;
	  ray.start.Z = zTestPos;
	  ray.end = ray.start;
	  ray.end.X = minX - 0.1;

	  // Check the ray and add the contact point if it exists
	  addContactPointFromRay(ray, ySpacing * zSpacing);
	  // swap ray direction and check again
	  ray.start.X = minX - 0.1;
	  ray.end.X = maxX + 0.1;
	  addContactPointFromRay(ray, ySpacing * zSpacing);
        }
    }

  // We don't want to do further triangle selection with the ship, so set the selector to null
  mShipScene->setTriangleSelector(0);
  mTriangleSelectorEnabled = false;
}

void Collision::addContactPointFromRay(irr::core::line3d<irr::f32> ray, irr::f32 contactArea)
{
  irr::core::vector3df intersection;
  irr::core::triangle3df hitTriangle;

  irr::scene::ISceneNode *selectedSceneNode =
    mDevice->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
												 ray,
												 intersection,      // This will be the position of the collision
												 hitTriangle,       // This will be the triangle hit in the collision
												 IDFlag_IsPickable, // (bitmask)
												 0);                // Check all nodes

  if (selectedSceneNode)
    {
      ContactPoint contactPoint;
      contactPoint.position = intersection;
      contactPoint.normal = hitTriangle.getNormal().normalize();
      contactPoint.position.Y -= mHeightCorr; // Adjust for height correction

      // Check if the normal is pointing 'towards' the incoming ray used to find the contact point, i.e. if it is pointing in roughly the right direction
      // 0.707 is approximately cos(45deg), so should be within +- 45 degrees of the incoming ray.
      if (contactPoint.normal.dotProduct(ray.getVector().normalize()) < -0.707)
        {

	  // Find an internal node position, i.e. a point at which a ray check for internal intersection can start
	  ray.start = contactPoint.position;
	  // leave ray.end as the same as before
	  // Check for the internal node
	  selectedSceneNode =
	    mDevice->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
													 ray,
													 intersection,      // This will be the position of the collision
													 hitTriangle,       // This will be the triangle hit in the collision
													 IDFlag_IsPickable, // (bitmask)
													 0);                // Check all nodes

	  if (selectedSceneNode)
            {
	      contactPoint.internalPosition = intersection;
	      contactPoint.internalPosition.Y -= mHeightCorr; // Adjust for height correction

	      // Adjust internal position, so it's only 1/2 way to the opposite boundary of the model
	      contactPoint.internalPosition = 0.5 * contactPoint.internalPosition + 0.5 * contactPoint.position;

	      // Find cross product, for torque component
	      irr::core::vector3df crossProduct = contactPoint.position.crossProduct(contactPoint.normal);
	      contactPoint.torqueEffect = crossProduct.Y;

	      // Store effective area represented by the contact
	      contactPoint.effectiveArea = contactArea;

	      // Store the contact point that we have found
	      contactPoints.push_back(contactPoint); // Store
            }
        }
    }
}


void Collision::enableTriangleSelector(bool aSelectorEnabled)
{

  // Only re-set if we need to change the state

  if (aSelectorEnabled && !mTriangleSelectorEnabled)
    {
      mShipScene->setTriangleSelector(mSelector);
      mTriangleSelectorEnabled = true;
    }

  if (!aSelectorEnabled && mTriangleSelectorEnabled)
    {
      mShipScene->setTriangleSelector(0);
      mTriangleSelectorEnabled = false;
    }
}


void Collision::DetectAndRespond(irr::f32 &reaction, irr::f32 &lateralReaction, irr::f32 &turnReaction)
{
  bool debug=false;
 
  reaction = 0;
  lateralReaction = 0;
  turnReaction = 0;

  mBuoyCollision = false;
  mOtherShipCollision = false;

  // Normal ship model
  mShipScene->updateAbsolutePosition();
  irr::core::matrix4 rot;
  rot.setRotationDegrees(mShipScene->getRotation());
  irr::core::vector3df shipAbsolutePosition = mShipScene->getAbsolutePosition();

  for (int i = 0; i < contactPoints.size(); i++)
    {
      irr::core::vector3df pointPosition = contactPoints.at(i).position;
      irr::core::vector3df pointPositionForNormal = pointPosition + contactPoints.at(i).normal;
      irr::core::vector3df internalPointPosition = contactPoints.at(i).internalPosition;

      // Rotate with own ship
      rot.transformVect(pointPosition);
      rot.transformVect(pointPositionForNormal);
      rot.transformVect(internalPointPosition);

      pointPosition += shipAbsolutePosition;
      pointPositionForNormal += shipAbsolutePosition;
      internalPointPosition += shipAbsolutePosition;

      irr::f32 localIntersection = 0; // Ready to use

      // Find depth below the contact point
      irr::f32 localDepth = -1 * mModel->getTerrain()->getHeight(pointPosition.X, pointPosition.Z) + pointPosition.Y;

      // Contact model (proof of principle!)
      if (localDepth < 0)
	{
	  localIntersection = -1 * localDepth * std::abs(contactPoints.at(i).normal.Y); // Projected based on normal, so we get an estimate of the intersection normal to the contact point. Ideally this vertical component of the normal would react to the ship's motion, but probably not too important
	}

      irr::f32 remotePointAxialSpeed = 0;
      irr::f32 remotePointLateralSpeed = 0;

      // Also check contact with pickable scenery elements here (or other ships?)
      irr::core::line3d<irr::f32> ray(internalPointPosition, pointPosition);
      irr::core::vector3df intersection;
      irr::core::triangle3df hitTriangle;
      irr::scene::ISceneNode *selectedSceneNode =
	mDevice->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
												     ray,
												     intersection,      // This will be the position of the collision
												     hitTriangle,       // This will be the triangle hit in the collision
												     IDFlag_IsPickable, // (bitmask)
												     0);                // Check all nodes

      // Check normal directions of contact triangle  - if they are pointing in the same direction, then we are on the 'free' side of the contact, and can ignore it
      if (selectedSceneNode)
	{
	  // First find the normal of the contact point (on the ship) in world coordinates
	  irr::core::vector3df worldCoordsShipNormal = pointPositionForNormal - pointPosition;
	  if (hitTriangle.getNormal().dotProduct(worldCoordsShipNormal) > 0)
	    {
	      // Ignore this contact
	      selectedSceneNode = 0;
	    }
	}

      // If this returns something, we must be in contact, so find distance between intersection and pointPosition
      if (selectedSceneNode && std::string(selectedSceneNode->getName()).find("LandObject") == 0)
	{

	  irr::f32 collisionDistance = pointPosition.getDistanceFrom(intersection);

	  // If we're more collided with an object than the terrain, use this
	  if (collisionDistance > localIntersection)
	    {
	      localIntersection = collisionDistance;
	    }
	}

      // Also check for buoy collision
      if (selectedSceneNode && std::string(selectedSceneNode->getName()).find("Buoy") == 0)
	{
	  mBuoyCollision = true;
	}

      // And for other ship collision
      if (selectedSceneNode && std::string(selectedSceneNode->getName()).find("OtherShip") == 0)
	{
	  mOtherShipCollision = true;

	  irr::s32 otherShipID = -1;
	  // Find other ship ID from name (should be OtherShip_#)
	  std::vector<std::string> splitName = Utilities::split(std::string(selectedSceneNode->getName()), '_');
	  if (splitName.size() == 2)
	    {
	      otherShipID = Utilities::lexical_cast<irr::s32>(splitName.at(1));
	    }
	  // std::cout << "In contact with " << std::string(selectedSceneNode->getName()) << " Length of split: " << splitName.size() << std::endl;

	  // Testing: behave as if other ship is solid. In multiplayer, the other ship (if another 'player') should also respond
	  irr::f32 collisionDistance = pointPosition.getDistanceFrom(intersection);
	  // If we're more collided with an object than the terrain, use this
	  if (collisionDistance > localIntersection)
	    {
	      localIntersection = collisionDistance;

	      // Calculate velocity of other ship, in our reference frame
	      if (otherShipID >= 0)
		{
		  irr::f32 otherShipHeading = mModel->getOtherShips()->getHeading(otherShipID);
		  irr::f32 otherShipSpeed = mModel->getOtherShips()->getSpeed(otherShipID);
		  irr::f32 otherShipRelativeHeading = otherShipHeading - mModel->getOwnShip()->getLateralSpeed();

		  // TODO: Initially ignore rate of turn of other ship, but should be included
		  remotePointAxialSpeed = otherShipSpeed * cos(irr::core::DEGTORAD * otherShipRelativeHeading);
		  remotePointLateralSpeed = otherShipSpeed * sin(irr::core::DEGTORAD * otherShipRelativeHeading);
		}
	    }
	}

      // Contact model (proof of principle!)
      if (localIntersection > 5)
	{
	  localIntersection = 5; // Limit to 5m intersection
	}

      if (localIntersection > 0)
	{
	  // Simple 'proof of principle' values initially
	  // reaction += localIntersection*100*maxForce * sign(axialSpd,0.1);
	  // lateralReaction += localIntersection*100*maxForce * sign(lateralSpd,0.1);
	  // turnReaction += localIntersection*100*maxForce * sign(rateOfTurn,0.1);

	  // Find effective area of contact point
	  irr::f32 contactArea = contactPoints.at(i).effectiveArea;

	  // Define stiffness & damping
	  irr::f32 contactStiffness = mModel->getModelParameters().contactStiffnessFactor * contactArea;                         // N/m per m2 * area
	  irr::f32 contactDamping = mModel->getModelParameters().contactDampingFactor * 2.0 * sqrt(contactStiffness * mModel->getOwnShip()->getM()); // Critical damping, assuming that only one point is in contact, and that mass of own ship is the smaller in two body contact...

	  // Local speed at this point (TODO, include y component from pitch and roll?)
	  //  Relative to the speed of the point we're in contact with
	  irr::core::vector3df localSpeedVector;
	  localSpeedVector.X = mModel->getOwnShip()->getLateralSpeed() + mModel->getOwnShip()->getRateOfTurn() * contactPoints.at(i).position.Z - remotePointLateralSpeed;
	  localSpeedVector.Y = 0;
	  localSpeedVector.Z = mModel->getOwnShip()->getSpeed() - mModel->getOwnShip()->getRateOfTurn() * contactPoints.at(i).position.X - remotePointAxialSpeed;

	  // Find the speed component, tangential to the contact plane (for friction)
	  irr::core::vector3df tangentialSpeedComponent;
	  // Find this here, by subtracting the part normal to the contact plane
	  // part normal to the contact plane is speedVector.normal * normal (normal is already normalised length)
	  tangentialSpeedComponent = localSpeedVector - localSpeedVector.dotProduct(contactPoints.at(i).normal) * contactPoints.at(i).normal;

	  irr::f32 tangentialSpeedAmplitude = tangentialSpeedComponent.getLength();
	  irr::core::vector3df normalisedTangentialSpeedComponent = tangentialSpeedComponent; // Normalised, so we just have the direction
	  normalisedTangentialSpeedComponent.normalize();

	  // Simple 'stiffness' based response
	  irr::f32 reactionForce = localIntersection * contactStiffness;
	  // Damping: Project localSpeedVector onto contact normal. Damping reaction force is proportional to this, and can be applied like the main reaction force
	  irr::f32 normalSpeed = localSpeedVector.dotProduct(contactPoints.at(i).normal);
	  irr::f32 dampingForce = normalSpeed * contactDamping;

	  // Find combined stiffness and damping effect. Only allow to be positive, so no 'sticking'
	  irr::f32 combinedStiffnessDamping = reactionForce + dampingForce;
	  if (combinedStiffnessDamping < 0.0)
	    {
	      combinedStiffnessDamping = 0.0;
	    }

	  // Apply this force
	  turnReaction += combinedStiffnessDamping * contactPoints.at(i).torqueEffect;
	  reaction += combinedStiffnessDamping * contactPoints.at(i).normal.Z;
	  lateralReaction += combinedStiffnessDamping * contactPoints.at(i).normal.X;

	  // Friction response. Use tanh function for better stability at low speed
	  irr::f32 frictionTorqueFactor = (contactPoints.at(i).position.crossProduct(normalisedTangentialSpeedComponent)).Y; // Effect of unit friction force on ship's turning. TODO: Check this, I think it's correct
	  irr::f32 frictionCoeff = mModel->getModelParameters().frictionCoefficient * tanh(mModel->getModelParameters().tanhFrictionFactor * tangentialSpeedAmplitude);
	  turnReaction += combinedStiffnessDamping * frictionCoeff * frictionTorqueFactor;
	  reaction += combinedStiffnessDamping * frictionCoeff * normalisedTangentialSpeedComponent.Z;
	  lateralReaction += combinedStiffnessDamping * frictionCoeff * normalisedTangentialSpeedComponent.X;

	  // std::cout << "remotePointAxialSpeed: " << remotePointAxialSpeed << std::endl;

	  if (debug)
	    {
	      // Show points in contact in red
	      irr::core::position2d<irr::s32> contactPoint2d = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																			  pointPosition, mDevice->getSceneManager()->getActiveCamera(), false);
	      irr::core::position2d<irr::s32> contactPoint2dNormal = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																				pointPositionForNormal, mDevice->getSceneManager()->getActiveCamera(), false);
	      mDevice->getVideoDriver()->draw2DPolygon(contactPoint2d, 10, irr::video::SColor(100, 255, 0, 0));
	      mDevice->getVideoDriver()->draw2DLine(contactPoint2d, contactPoint2dNormal, irr::video::SColor(100, 255, 0, 0));
	    }
	}
      else
	{
	  if (debug)
	    {
	      // Show points not in contact
	      irr::video::SColor pointColour;
	      if (contactPoints.at(i).torqueEffect > 0)
		{
		  pointColour = irr::video::SColor(100, 0, 255, 0);
		}
	      else
		{
		  pointColour = irr::video::SColor(100, 0, 0, 255);
		}

	      irr::core::position2d<irr::s32> contactPoint2d = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																			  pointPosition, mDevice->getSceneManager()->getActiveCamera(), false);
	      irr::core::position2d<irr::s32> contactPoint2dNormal = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																				pointPositionForNormal, mDevice->getSceneManager()->getActiveCamera(), false);
	      mDevice->getVideoDriver()->draw2DPolygon(contactPoint2d, 10, pointColour);
	      mDevice->getVideoDriver()->draw2DLine(contactPoint2d, contactPoint2dNormal, pointColour);
	    }
	}
      // contactDebugPoints.at(i*2)->setPosition(internalPointPosition);
      // contactDebugPoints.at(i*2 + 1)->setPosition(internalPointPosition);
    }

  // If showing debug data, draw a big circle series for the model centre
  if (debug)
    {
      irr::core::position2d<irr::s32> centrePosition2d = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																		    mShipScene->getAbsolutePosition(), mDevice->getSceneManager()->getActiveCamera(), false);
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 5, irr::video::SColor(100, 0, 255, 0));
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 10, irr::video::SColor(100, 0, 255, 0));
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 15, irr::video::SColor(100, 0, 255, 0));
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 20, irr::video::SColor(100, 0, 255, 0));
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 25, irr::video::SColor(100, 0, 255, 0));
    }
}


irr::scene::ISceneNode* Collision::getContactFromRay(irr::core::line3d<float> ray, irr::s32 linesMode)
{

  // Temporarily enable all required triangle selectors
  if (linesMode == 1) {
    // Start - on own ship
    enableTriangleSelector(true);
  } else if (linesMode == 2) {
    // End - not on own ship
    mModel->getOtherShips()->enableAllTriangleSelectors(); //This will be reset next time otherShips.update is called
    mModel->getBuoys()->enableAllTriangleSelectors(); //This will be reset next time otherShips.update is called
    // TODO: Temporarily enable triangle selector for:
    //   Terrain
    //   Land objects
  } else {
    // Not start or end, return null;
    return 0;
  }

  irr::core::vector3df intersection;
  irr::core::triangle3df hitTriangle;

  irr::scene::ISceneNode * selectedSceneNode =
    mSmgr->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
									    ray,
									    intersection, // This will be the position of the collision
									    hitTriangle, // This will be the triangle hit in the collision
									    IDFlag_IsPickable, // (bitmask), 0 for all
									    0); // Check all nodes

  irr::scene::ISceneNode* contactPointNode = 0;

  if (selectedSceneNode &&
      (
       ((linesMode == 1) && (selectedSceneNode == mShipScene)) || // Valid start node
       ((linesMode == 2) && (selectedSceneNode != mShipScene))    // Valid end node
       )
      ) {

    // Add a 'sphere' scene node, with selectedSceneNode as parent.
    // Find local coordinates from the global one
    irr::core::vector3df localPosition(intersection);
    irr::core::matrix4 worldToLocal = selectedSceneNode->getAbsoluteTransformation();
    worldToLocal.makeInverse();
    worldToLocal.transformVect(localPosition);

    irr::core::vector3df sphereScale = irr::core::vector3df(1.0, 1.0, 1.0);
    if (selectedSceneNode && selectedSceneNode->getScale().X > 0) {
      sphereScale = irr::core::vector3df(1.0f/selectedSceneNode->getScale().X,
					 1.0f/selectedSceneNode->getScale().X,
					 1.0f/selectedSceneNode->getScale().X);
    }

    contactPointNode = mSmgr->addSphereSceneNode(0.25f,16,selectedSceneNode,-1,
						 localPosition,
						 irr::core::vector3df(0, 0, 0),
						 sphereScale);

    // Set name to match parent for convenience
    contactPointNode->setName(selectedSceneNode->getName());
  }

  // Reset triangle selectors
  enableTriangleSelector(false); // Own ship should not need triangle selectors at runtime (todo: for future robustness, check previous state and restore to this)
  // buoys and otherShips will be reset when their update() method is called

  return contactPointNode;
}
