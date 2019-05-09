#include<string>

#include"chunk.h"

using std::string; using std::to_string;

const char * getName(string);

const float Chunk::blockSize = 1.0f;

unsigned char Chunk::getHeight(unsigned char x, unsigned char y) {
	return 128 + (x + y) % 2;
}

Chunk::Chunk(Ogre::SceneManager *man, Ogre::Root * r, const Ogre::Vector3 & v) : mManager(man), mRoot(r) {
	mManager->getRootSceneNode()->createChildSceneNode(v);
	for (int x = 0; x < 255; ++x) for (int y = 0; y < 255; ++y) {
		unsigned char h = getHeight(x, y);
		for (int z = 0; z < 255; ++z) {
			blocks[0xff * 0xff * x + 0xff * y + z] = z <= h ? BlockType::stone : BlockType::none;
		}
	}
	updateChunkMesh();
};

void Chunk::updateChunkMesh() {
	if (!mNode) return;

	for (int x = 0; x <= 255; ++x) for (int y = 0; y <= 255; ++y) for (int z = 0; z <= 255; ++z) {
		//Basic idea: generate a face iff the face is on the outside of the chunk or the face is not adjacent to any block
		
		bool x1 = false, x2 = false, z1 = false, z2 = false, y = false;
		Ogre::Vector3 pos(x * 20, y * 15, z * 20);

		if (x==0 || isEmpty(x - 1, y, z)) {
			//Generate -x face
			x1 = true;
		}
		if (x == 255 || isEmpty(x + 1, y, z)) {
			//Generate +x face
			x2 = true;
		}
		if (y == 0 || isEmpty(x, y - 1, z)) {
			//Generate -y face
			y = true;
		}
		// if (y == 255 || isEmpty(x, y + 1, z)) {
			//Generate +y face
		// }
		if (z == 0 || isEmpty(x, y, z - 1)) {
			//Generate -z face
			z1 = true;
		}
		if (z == 255 || isEmpty(x, y, z + 1)) {
			//Generate +z face
			z2 = true;
		}

		Ogre::ManualObject *cube = createCubeMesh(getName("plane"), x1, x2, z1, z2, y);
		attachGeometry(cube, pos);

	}
	//mNode->attachObject(manObj);
}

Ogre::SceneNode * Chunk::attachGeometry(Ogre::ManualObject * myGeo, const Ogre::Vector3 & pos){
	Ogre::SceneNode * node = mNode->createChildSceneNode(pos);
	node->setOrientation(Ogre::Quaternion(Ogre::Degree(180), Ogre::Vector3(1, 0, 0)));
	node->attachObject(myGeo);

	return node;
}

const char * getName(string prefix){
	static unsigned int * i;
	if(!i)
		i = new unsigned int(0);
	
	return (prefix + to_string((*i)++)).c_str();
}

Ogre::ManualObject* Chunk::createCubeMesh(Ogre::String name, bool x1, bool x2, bool z1, bool z2, bool y) {

	Ogre::MaterialPtr matptr = Ogre::MaterialManager::getSingleton().create("BaseColoured", "General");
	matptr->load();
	matptr->getBestTechnique()->getPass(0)->setVertexColourTracking(Ogre::TVC_DIFFUSE);
	matptr->getBestTechnique()->getPass(0)->setLightingEnabled(false);

	Ogre::ManualObject* cube = new Ogre::ManualObject(name);

	float s = 20;

	cube->begin("BaseColoured", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	Ogre::Vector3 vec(s / 2, 0, 0);
	Ogre::Quaternion quat;
	quat.FromAngleAxis(Ogre::Degree(45), Ogre::Vector3::UNIT_Y);

	for (int i = 0; i < 8; i++) {
		cube->position(-vec.x, 15, -vec.z); cube->colour(0.78, 0.78, 0.78); cube->textureCoord(0, 0);
		cube->position(vec.x, 15, vec.z); cube->colour(0.78, 0.78, 0.78); cube->textureCoord(1, 0);
		cube->position(-vec.x, 0, -vec.z); cube->colour(0.78, 0.78, 0.78); cube->textureCoord(0, 1);
		cube->position(vec.x, 0, vec.z); cube->colour(0.78, 0.78, 0.78); cube->textureCoord(1, 1);

		//int offset = 4 * i;
		//cube->triangle(offset + 0, offset + 3, offset + 1);
		//cube->triangle(offset + 0, offset + 2, offset + 3);

		vec = quat * vec;
	}

	if (y)
		// y+
		cube->triangle(6, 15, 14); cube->triangle(7, 14, 15);
	if (x1)
		// x-
		cube->triangle(20, 30, 28); cube->triangle(22, 30, 20);
	if (x2)
		// x+
		cube->triangle(21, 31, 29); cube->triangle(23, 31, 21);
	if (z1)
		// z-
		cube->triangle(23, 28, 30); cube->triangle(21, 28, 23);
	if (z2)
		// z+
		cube->triangle(7, 12, 14); cube->triangle(5, 12, 7);

	cube->end();

	return cube;
}