#pragma once
#include "OgreManualObject.h"
#include "BaseApplication.h"

enum class BlockType {
	none,
	grass,
	dirt,
	stone
};

class Chunk {
public:
	Chunk();
	Chunk(Ogre::SceneManager *, Ogre::Root *, const Ogre::Vector3 &);

	void setBlock(unsigned char x, unsigned char y, unsigned char z, BlockType t) {
		blocks[0xff * 0xff * x + 0xff * y + z] = t;
		updateChunkMesh();
	}
	void setBlock(Ogre::Vector3 v, BlockType t) {
		if (!mNode) return;
		Ogre::Vector3 r = v - mNode->getPosition();
		setBlock(r.x, r.y, r.z, t);
	}
private:
	const static float blockSize;
	unsigned char getHeight(unsigned char, unsigned char);
	void updateChunkMesh();

	BlockType getBlock(unsigned char x, unsigned char y, unsigned char z) { return blocks[0xff * 0xff * x + 0xff * y + z]; }
	bool isEmpty(unsigned char x, unsigned char y, unsigned char z) { return getBlock(x, y, z) == BlockType::none; }

	Ogre::SceneManager * mManager;
	Ogre::SceneNode * mNode;
	Ogre::Root * mRoot;
	Ogre::ManualObject * manObj;

	BlockType blocks[0xff * 0xff * 0xff];
};