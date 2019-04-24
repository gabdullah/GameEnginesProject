#include"chunk.h"

const float Chunk::blockSize = 1.0f;

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

	//Clear the object
	if (manObj) {
		mNode->detachObject(manObj);
		delete manObj;
		manObj = mManager->createManualObject();
	}

	for (int x = 0; x < 255; ++x) for (int y = 0; y < 255; ++y) for (int z = 0; z < 0; ++z) {
		//Basic idea: generate a face iff the face is on the outside of the chunk or the face is not adjacent to any block
		if (x==0 || isEmpty(x - 1, y, z)) {
			//Generate -x face
		}
		if (x == 255 || isEmpty(x + 1, y, z)) {
			//Generate +x face
		}
		if (y == 0 || isEmpty(x, y - 1, z)) {
			//Generate -y face
		}
		if (y == 255 || isEmpty(x, y + 1, z)) {
			//Generate +y face
		}
		if (z == 0 || isEmpty(x, y, z - 1)) {
			//Generate -z face
		}
		if (z == 255 || isEmpty(x, y, z + 1)) {
			//Generate +z face
		}
	}
	mNode->attachObject(manObj);
}