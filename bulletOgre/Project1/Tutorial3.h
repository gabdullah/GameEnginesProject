#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include "OgreManualObject.h"
#include "btBulletDynamicsCommon.h"
#include "btHeightfieldTerrainShape.h"
#include "BaseApplication.h"
#include "flightPhysics.hpp"
#include "chunk.h"
 
class TutorialApplication : public BaseApplication
{
public:
  TutorialApplication();
  virtual ~TutorialApplication();
 
protected:
  virtual void createScene();
  virtual void createFrameListener();
  virtual void destroyScene();
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);
  bool frameStarted(const Ogre::FrameEvent &evt);
  void CreatePlane(const btVector3 &Position, btScalar Mass, const btVector3 &scale);
  //Ogre::ManualObject* TutorialApplication::createCubeMesh(Ogre::String name);

private:
  void input(const Ogre::FrameEvent& fe);
  void defineTerrain(long x, long y);
  void initBlendMaps(Ogre::Terrain* terrain);
  void configureTerrainDefaults(Ogre::Light* light);
  void createBulletSim(void);
  bool mTerrainsImported;
  Ogre::TerrainGroup* mTerrainGroup;
  Ogre::TerrainGlobalOptions* mTerrainGlobals;
 
  OgreBites::Label* mInfoLabel;
  btDefaultCollisionConfiguration* collisionConfiguration;
  btCollisionDispatcher* dispatcher;
  btBroadphaseInterface* overlappingPairCache;
  btSequentialImpulseConstraintSolver* solver;
  btDiscreteDynamicsWorld* dynamicsWorld;
  btCollisionShape* groundShape;
  btAlignedObjectArray<btCollisionShape*> collisionShapes;
  FlightPhyics flightPhyics;
};
 

