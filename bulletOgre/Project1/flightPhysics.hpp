#include "btBulletDynamicsCommon.h"

class FlightPhyics{
    FlightPhyics(btRigidBody * rb, float c_d, float c_d, float thrust): 
        mRigidBody(rb), 
        mC_d(c_d), 
        mC_l(c_l),
        max_thrust(thrust) {};
    operator()(float dt, float controls[3], float throttle);
private:
    btVector3 getForward();
    btVector3 getUp();
    btRigidBody * mRigidBody;
    float mC_d;
    float mC_l;
    float controlSurfaces[3];
    float max_thrust;
};