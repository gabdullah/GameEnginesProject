#include"flightPhysics.hpp"

void FlightPhyics::operator()(float dt, float controls[3], float throttle){
    btVector3 flightDir = getForward();
    btVector3 liftDir = getUp();
    btVector3 starboard = flightDir.cross(liftDir);

	float v = mRigidBody->getLinearVelocity().length();
    btVector3 vSqInFlight = mRigidBody->getLinearVelocity().dot(flightDir)*v

    btVector3 lift = liftDir * (mC_l * vSqInFlight);
    btVector3 drag = flightDir * (mC_d * v * v) * (-1);
    btVector3 thrust = flightDir * max_thrust * throttle;
    mRigidBody->applyCentralImpulse((thrust+drag+lift) * dt);

    btVector3 pitch = starboard * controlSurfaces[0] * controls[0];
    btVector3 roll = flightDir * controlSurfaces[1] * controls[1];
    btVector3 yaw = liftDir * controlSurfaces[2] * controls[2];

    mRigidBody->applyTorqueImpulse((pitch + roll + yaw) * dt);
}

btVector3 FlightPhyics::getForward(){
	btTransform trans;
	mRigidBody->getMotionState()->getWorldTransform(trans);
	return quatRotate(trans.getRotation().inverse(), (btVector3(0, 0, 1))); //May need to change this to another unit vector;
}

btVector3 FlightPhyics::getUp(){
    btTransform trans;
    mRigidBody->getMotionState()->getWorldTransform(trans);
    return quatRotate(trans.getRotation().inverse(),(btVector3(0,1,0))); //May need to change this to another unit vector;
}