#include "Tutorial3.h"
class MyMotionState : public btMotionState {
public:
	MyMotionState(const btTransform &initialpos, Ogre::SceneNode *node) {
		mVisibleobj = node;
		mPos1 = initialpos;
	}
	virtual ~MyMotionState() {    }
	void setNode(Ogre::SceneNode *node) {
		mVisibleobj = node;
	}
	virtual void getWorldTransform(btTransform &worldTrans) const {
		worldTrans = mPos1;
	}
	virtual void setWorldTransform(const btTransform &worldTrans) {
		if (NULL == mVisibleobj) return; // silently return before we set a node
		btQuaternion rot = worldTrans.getRotation();
		mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
		btVector3 pos = worldTrans.getOrigin();
		// TODO **** XXX need to fix this up such that it renders properly since this doesnt know the scale of the node
		// also the getCube function returns a cube that isnt centered on Z
		mVisibleobj->setPosition(pos.x(), pos.y() + 5, pos.z() - 5);
	}
protected:
	Ogre::SceneNode *mVisibleobj;
	btTransform mPos1;
};




TutorialApplication::TutorialApplication()
	: mTerrainGroup(0),
	mTerrainGlobals(0),
	mInfoLabel(0)
{
}

TutorialApplication::~TutorialApplication()
{
}

void TutorialApplication::CreatePlane(const btVector3 &Position, btScalar Mass, const btVector3 &scale){
	Ogre::Vector3 size = Ogre::Vector3::ZERO;
	Ogre::Vector3 pos = Ogre::Vector3::ZERO;
	Ogre::Entity* planeEntity;
	Ogre::SceneNode* planeNode;

	pos.x = Position.getX();
	pos.y = Position.getY();
	pos.z = Position.getZ();
	planeEntity = mSceneMgr->createEntity("plane", "RZR-002.mesh");
	planeEntity->setCastShadows(true);
	planeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(pos.x, pos.y, pos.z));
	planeNode->attachObject(planeEntity);

	mCamera->setNearClipDistance(.1);
	Ogre::SceneNode* cam_node = planeNode->createChildSceneNode();
	cam_node->setPosition(0, 10, -20);
	cam_node->setOrientation(Ogre::Quaternion(Ogre::Degree(180), Ogre::Vector3(0, 1, 0)));
	cam_node->attachObject(mCamera);

	Ogre::AxisAlignedBox boundingB = planeEntity->getBoundingBox();
	//boundingB.scale(Ogre::Vector3(scale.getX(), scale.getY(), scale.getZ()));
	size = boundingB.getSize()*0.95f;
	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(Position);
	MyMotionState *MotionState = new MyMotionState(Transform, planeNode);

	btVector3 HalfExtents(size.x*0.5f, size.y*0.5f, size.z*0.5f);
	btCollisionShape *Shape = new btSphereShape(size.x*0.5f);
	btVector3 LocalInertia;
	Shape->calculateLocalInertia(1.0f, LocalInertia);
	btRigidBody *RigidBody = new btRigidBody(0.1f, MotionState, Shape, LocalInertia);

	// Store a pointer to the Ogre Node so we can update it later
	RigidBody->setUserPointer((void *)(planeNode));

	// Add it to the physics world
	dynamicsWorld->addRigidBody(RigidBody);
	collisionShapes.push_back(Shape);

	flightPhyics = FlightPhyics(RigidBody, 0.0001f, 0.001f, 20.0f);
}
void TutorialApplication::createBulletSim(void) {
	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new   btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -100, 0));
	{
		///create a few basic rigid bodies
		// start with ground plane, 1500, 1500
		Ogre::Terrain * pTerrain = mTerrainGroup->getTerrain(0, 0);
		float* terrainHeightData = pTerrain->getHeightData();
		Ogre::Vector3 terrainPosition = pTerrain->getPosition();
		float * pDataConvert = new float[pTerrain->getSize() *pTerrain->getSize()];
		for (int i = 0; i < pTerrain->getSize(); i++)
			memcpy(
				pDataConvert + pTerrain->getSize() * i, // source
				terrainHeightData + pTerrain->getSize() * (pTerrain->getSize() - i - 1), // target
				sizeof(float)*(pTerrain->getSize()) // size
			);

		float metersBetweenVertices = pTerrain->getWorldSize() / (pTerrain->getSize() - 1); //edit: fixed 0 -> 1 on 2010-08-13
		btVector3 localScaling(metersBetweenVertices, 1, metersBetweenVertices);

		btHeightfieldTerrainShape* groundShape = new btHeightfieldTerrainShape(
			pTerrain->getSize(),
			pTerrain->getSize(),
			pDataConvert,
			1/*ignore*/,
			pTerrain->getMinHeight(),
			pTerrain->getMaxHeight(),
			1,
			PHY_FLOAT,
			true);

		groundShape->setUseDiamondSubdivision(true);
		groundShape->setLocalScaling(localScaling);

		btRigidBody * mGroundBody = new btRigidBody(0, new btDefaultMotionState(), groundShape);

		mGroundBody->getWorldTransform().setOrigin(
			btVector3(
				terrainPosition.x,
				terrainPosition.y + (pTerrain->getMaxHeight() - pTerrain->getMinHeight()) / 2,
				terrainPosition.z));

		mGroundBody->getWorldTransform().setRotation(
			btQuaternion(
				Ogre::Quaternion::IDENTITY.x,
				Ogre::Quaternion::IDENTITY.y,
				Ogre::Quaternion::IDENTITY.z,
				Ogre::Quaternion::IDENTITY.w));

		dynamicsWorld->addRigidBody(mGroundBody);
		collisionShapes.push_back(groundShape);

		CreatePlane(btVector3(2063, 1500, 1250), 1.0f, btVector3(1.2, 1.2, 1.2));
	}
}

void TutorialApplication::createScene()
{
	/*mCamera->setPosition(Ogre::Vector3(1863, 60, 1650));
	mCamera->lookAt(Ogre::Vector3(2263, 50, 1200));
	mCamera->setNearClipDistance(.1);*/

	bool infiniteClip =
		mRoot->getRenderSystem()->getCapabilities()->hasCapability(
			Ogre::RSC_INFINITE_FAR_PLANE);

	/*if (infiniteClip)
		mCamera->setFarClipDistance(0);
	else
		mCamera->setFarClipDistance(50000);*/

	//Chunk chunk(mSceneMgr, mRoot, Ogre::Vector3::ZERO);

	mSceneMgr->setAmbientLight(Ogre::ColourValue(.2, .2, .2));

	Ogre::Vector3 lightDir(.55, -.3, .75);
	lightDir.normalise();

	Ogre::Light* light = mSceneMgr->createLight("TestLight");
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(lightDir);
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue(.4, .4, .4));

	// Fog
	Ogre::ColourValue fadeColour(.9, .9, .9);
	mWindow->getViewport(0)->setBackgroundColour(fadeColour);

	//mSceneMgr->setFog(Ogre::FOG_EXP2, fadeColour, 0.002);

	  // Terrain
	mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();

	mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(
		mSceneMgr,
		Ogre::Terrain::ALIGN_X_Z,
		513, 12000.0);
	mTerrainGroup->setFilenameConvention(Ogre::String("terrain"), Ogre::String("dat"));
	mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);

	configureTerrainDefaults(light);

	for (long x = 0; x <= 0; ++x)
		for (long y = 0; y <= 0; ++y)
			defineTerrain(x, y);

	mTerrainGroup->loadAllTerrains(true);

	if (mTerrainsImported)
	{
		Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();

		while (ti.hasMoreElements())
		{
			Ogre::Terrain* t = ti.getNext()->instance;
			initBlendMaps(t);
		}
	}

	mTerrainGroup->freeTemporaryResources();

	// Sky Techniques
	// mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 300, false);
	mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);
	// Ogre::Plane plane;
	// plane.d = 1000;
	// plane.normal = Ogre::Vector3::NEGATIVE_UNIT_Y;

	// mSceneMgr->setSkyPlane(
	//   true, plane, "Examples/SpaceSkyPlane", 1500, 40, true, 1.5, 150, 150);
	createBulletSim();
	
	
}

void TutorialApplication::createFrameListener()
{
	BaseApplication::createFrameListener();

	mInfoLabel = mTrayMgr->createLabel(OgreBites::TL_TOP, "TerrainInfo", "", 350);
}

void TutorialApplication::destroyScene()
{
	OGRE_DELETE mTerrainGroup;
	OGRE_DELETE mTerrainGlobals;
}

bool TutorialApplication::frameStarted(const Ogre::FrameEvent &evt)
{
	//	mKeyboard->capture();
	//	mMouse->capture();
		// update physics simulation
		//dynamicsWorld->stepSimulation(evt.timeSinceLastFrame,10);
	dynamicsWorld->stepSimulation(evt.timeSinceLastFrame);
	return true;
}

bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& fe)
{
	bool ret = BaseApplication::frameRenderingQueued(fe);

	if (mTerrainGroup->isDerivedDataUpdateInProgress())
	{
		mTrayMgr->moveWidgetToTray(mInfoLabel, OgreBites::TL_TOP, 0);
		mInfoLabel->show();

		if (mTerrainsImported)
			mInfoLabel->setCaption("Building terrain...");
		else
			mInfoLabel->setCaption("Updating textures...");
	}
	else
	{
		mTrayMgr->removeWidgetFromTray(mInfoLabel);
		mInfoLabel->hide();

		if (mTerrainsImported)
		{
			mTerrainGroup->saveAllTerrains(true);
			mTerrainsImported = false;
		}
	}

	input(fe);

	return ret;
}

void TutorialApplication::input(const Ogre::FrameEvent& fe) {
	float dirVec[3] = { 0, 0, 0 };

	if (mKeyboard->isKeyDown(OIS::KC_W))
		dirVec[0] -= 1;
	if (mKeyboard->isKeyDown(OIS::KC_S))
		dirVec[0] += 1;
	if (mKeyboard->isKeyDown(OIS::KC_Q))
		dirVec[1] -= 1;
	if (mKeyboard->isKeyDown(OIS::KC_E))
		dirVec[1] += 1;
	if (mKeyboard->isKeyDown(OIS::KC_A))
		dirVec[2] += 1;
	if (mKeyboard->isKeyDown(OIS::KC_D))
		dirVec[2] -= 1;

	flightPhyics(fe.timeSinceLastFrame, dirVec, 1.0f);
}

void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
{
	img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	if (flipX)
		img.flipAroundY();
	if (flipY)
		img.flipAroundX();

}

void TutorialApplication::defineTerrain(long x, long y)
{
	Ogre::String filename = mTerrainGroup->generateFilename(x, y);

	bool exists =
		Ogre::ResourceGroupManager::getSingleton().resourceExists(
			mTerrainGroup->getResourceGroup(),
			filename);

	if (exists)
		mTerrainGroup->defineTerrain(x, y);
	else
	{
		Ogre::Image img;
		getTerrainImage(x % 2 != 0, y % 2 != 0, img);
		mTerrainGroup->defineTerrain(x, y, &img);

		mTerrainsImported = true;
	}
}

void TutorialApplication::initBlendMaps(Ogre::Terrain* terrain)
{
	Ogre::Real minHeight0 = 70;
	Ogre::Real fadeDist0 = 40;
	Ogre::Real minHeight1 = 70;
	Ogre::Real fadeDist1 = 15;

	Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
	Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);

	float* pBlend0 = blendMap0->getBlendPointer();
	float* pBlend1 = blendMap1->getBlendPointer();

	for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
	{
		for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
		{
			Ogre::Real tx, ty;

			blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
			Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
			Ogre::Real val = (height - minHeight0) / fadeDist0;
			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
			*pBlend0++ = val;

			val = (height - minHeight1) / fadeDist1;
			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
			*pBlend1++ = val;
		}
	}

	blendMap0->dirty();
	blendMap1->dirty();
	blendMap0->update();
	blendMap1->update();

}

void TutorialApplication::configureTerrainDefaults(Ogre::Light* light)
{
	mTerrainGlobals->setMaxPixelError(8);
	mTerrainGlobals->setCompositeMapDistance(3000);

	mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());

	Ogre::Terrain::ImportData& importData = mTerrainGroup->getDefaultImportSettings();
	importData.terrainSize = 513;
	importData.worldSize = 12000.0;
	importData.inputScale = 600;
	importData.minBatchSize = 33;
	importData.maxBatchSize = 65;

	importData.layerList.resize(3);
	importData.layerList[0].worldSize = 100;
	importData.layerList[0].textureNames.push_back(
		"dirt_grayrocky_diffusespecular.dds");
	importData.layerList[0].textureNames.push_back(
		"dirt_grayrocky_normalheight.dds");
	importData.layerList[1].worldSize = 30;
	importData.layerList[1].textureNames.push_back(
		"grass_green-01_diffusespecular.dds");
	importData.layerList[1].textureNames.push_back(
		"grass_green-01_normalheight.dds");
	importData.layerList[2].worldSize = 200;
	importData.layerList[2].textureNames.push_back(
		"growth_weirdfungus-03_diffusespecular.dds");
	importData.layerList[2].textureNames.push_back(
		"growth_weirdfungus-03_normalheight.dds");

}


#if Ogre_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char *argv[])
#endif
	{
		// Create application object
		TutorialApplication app;

		try {
			app.go();
		}
		catch (Ogre::Exception& e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occured: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}

		return 0;
	}

#ifdef __cplusplus
}
#endif