/***********************************************************************
CollisionBox - Class to represent a rectangular box containing spheres
of a fixed radius interacting by fully elastic collisions.
Copyright (c) 2005 Oliver Kreylos
***********************************************************************/

#define COLLISIONBOX_IMPLEMENTATION

#include <Math/Math.h>

#include "CollisionBox.h"

/*****************************
Methods of class CollisionBox:
*****************************/

template <class ScalarT, int dimN>
inline
void
CollisionBox<ScalarT, dimN>::queueCollisionsInCell(
    typename CollisionBox<ScalarT, dimN>::GridCell* cell,
    typename CollisionBox<ScalarT, dimN>::Particle* particle1,
    typename CollisionBox<ScalarT, dimN>::Scalar timeStep,
    bool symmetric,
    typename CollisionBox<ScalarT, dimN>::Particle* otherParticle,
    typename CollisionBox<ScalarT, dimN>::CollisionQueue& collisionQueue)
{
    /* Calculate all intersections between two particles: */
    for (Particle* particle2=cell->particlesHead;particle2!=0;particle2=particle2->cellSucc)
    {
        if (particle2!=particle1 && particle2!=otherParticle && (symmetric||particle2>particle1))
        {
            /* Calculate any possible intersection time between the two particles: */
            Vector d=particle1->position-particle2->position;
            d-=particle1->velocity*particle1->timeStamp;
            d+=particle2->velocity*particle2->timeStamp;
            Vector vd=particle1->velocity-particle2->velocity;
            Scalar vd2=Geometry::sqr(vd);
            if (vd2>Scalar(0)) // Are the two particles' velocities different?
            {
                /* Solve the quadratic equation determining possible collisions: */
                Scalar ph=(d*vd)/vd2;
                Scalar q=(Geometry::sqr(d)-Scalar(4)*particleRadius2)/vd2;
                Scalar det=Math::sqr(ph)-q;
                if (det>=Scalar(0)) // Are there any solutions?
                {
                    /* Calculate the first solution (only that can be valid): */
                    Scalar collisionTime=-ph-Math::sqrt(det);

                    /* If the collision is valid, i.e., occurs past the last update of both particles, queue it: */
                    if (collisionTime>particle1->timeStamp && collisionTime>particle2->timeStamp && collisionTime<=timeStep)
                        collisionQueue.insert(CollisionEvent(collisionTime,particle1,particle2));
                }
            }
        }
    }
}

template <class ScalarT, int dimN>
inline
void
CollisionBox<ScalarT, dimN>::queueCellChanges(
    typename CollisionBox<ScalarT, dimN>::Particle* particle,
    const typename CollisionBox<ScalarT, dimN>::Point& newPosition,
    typename CollisionBox<ScalarT, dimN>::Scalar timeStep,
    typename CollisionBox<ScalarT, dimN>::CollisionQueue& collisionQueue)
{
    /* Check for crossing of any cell borders: */
    GridCell* cell=particle->cell;
    Scalar cellChangeTime=timeStep;
    int cellChangeDirection=-1;
    for (int i=0;i<dimension;++i)
    {
        if (newPosition[i]<cell->boundaries.min[i])
        {
            Scalar collisionTime=particle->timeStamp+(cell->boundaries.min[i]-particle->position[i])/particle->velocity[i];
            if (cellChangeTime>collisionTime)
            {
                cellChangeTime=collisionTime;
                cellChangeDirection=2*i+0;
            }
        }
        else if (newPosition[i]>cell->boundaries.max[i])
        {
            Scalar collisionTime=particle->timeStamp+(cell->boundaries.max[i]-particle->position[i])/particle->velocity[i];
            if (cellChangeTime>collisionTime)
            {
                cellChangeTime=collisionTime;
                cellChangeDirection=2*i+1;
            }
        }
    }
    if (cellChangeDirection>=0)
        collisionQueue.insert(CollisionEvent(cellChangeTime,particle,cellChangeDirection));
}

template <class ScalarT, int dimN>
inline
void
CollisionBox<ScalarT, dimN>::queueCollisions(
    typename CollisionBox<ScalarT, dimN>::Particle* particle1,
    typename CollisionBox<ScalarT, dimN>::Scalar timeStep,
    bool symmetric,
    typename CollisionBox<ScalarT, dimN>::Particle* otherParticle,
    typename CollisionBox<ScalarT, dimN>::CollisionQueue& collisionQueue)
{
    /* Calculate the particle's position at the end of this time step: */
    Point newPosition=particle1->position+particle1->velocity*(timeStep-particle1->timeStamp);
    
    /* Check for crossing of cell borders: */
    queueCellChanges(particle1,newPosition,timeStep,collisionQueue);
    
    /* Check for collision with any of the collision box's walls: */
    for (int i=0;i<dimension;++i) {
        if (newPosition[i]<boundaries.min[i]+particleRadius) {
            Scalar collisionTime=particle1->timeStamp+(boundaries.min[i]+particleRadius-particle1->position[i])/particle1->velocity[i];
            if (collisionTime<particle1->timeStamp)
                collisionTime=particle1->timeStamp;
            else if (collisionTime>timeStep)
                collisionTime=timeStep;
            Vector wallNormal=Vector::zero;
            wallNormal[i]=Scalar(1);
            collisionQueue.insert(CollisionEvent(collisionTime,particle1,wallNormal));
        }
        else if (newPosition[i]>boundaries.max[i]-particleRadius) {
            Scalar collisionTime=particle1->timeStamp+(boundaries.max[i]-particleRadius-particle1->position[i])/particle1->velocity[i];
            if (collisionTime<particle1->timeStamp)
                collisionTime=particle1->timeStamp;
            else if (collisionTime>timeStep)
                collisionTime=timeStep;
            Vector wallNormal=Vector::zero;
            wallNormal[i]=Scalar(-1);
            collisionQueue.insert(CollisionEvent(collisionTime,particle1,wallNormal));
        }
    }
    
    /* Check for collision with the spherical obstacle: */
    Vector d=particle1->position-spherePosition;
    d-=particle1->velocity*particle1->timeStamp;
    d+=sphereVelocity*sphereTimeStamp;
    Vector vd=particle1->velocity-sphereVelocity;
    Scalar vd2=Geometry::sqr(vd);
    if (vd2>Scalar(0)) { // Are the two particles' velocities different?
        /* Solve the quadratic equation determining possible collisions: */
        Scalar ph=(d*vd)/vd2;
        Scalar q=(Geometry::sqr(d)-Math::sqr(particleRadius+sphereRadius))/vd2;
        Scalar det=Math::sqr(ph)-q;
        if (det>=Scalar(0)) { // Are there any solutions?
            /* Calculate the first solution (only that can be valid): */
            Scalar collisionTime=-ph-Math::sqrt(det);

            /* If the collision is valid, i.e., occurs past the last update of both particles, queue it: */
            if (collisionTime>particle1->timeStamp && collisionTime>sphereTimeStamp && collisionTime<=timeStep) {
                collisionQueue.insert(CollisionEvent(collisionTime,particle1,sphereTimeStamp));
            }
        }
    }
    
    /* Check for collision with any other particle: */
    GridCell* baseCell=particle1->cell;
    for (int i=0;i<numNeighbors;++i)
    {
        GridCell* cell=baseCell+neighborOffsets[i];
        queueCollisionsInCell(cell,particle1,timeStep,symmetric,otherParticle,collisionQueue);
    }
}

template <class ScalarT, int dimN>
inline
void
CollisionBox<ScalarT, dimN>::queueCollisionsOnCellChange(
    typename CollisionBox<ScalarT, dimN>::Particle* particle,
    typename CollisionBox<ScalarT, dimN>::Scalar timeStep,
    int cellChangeDirection,
    typename CollisionBox<ScalarT, dimN>::CollisionQueue& collisionQueue)
{
    /* Calculate the particle's position at the end of this time step: */
    Point newPosition=particle->position+particle->velocity*(timeStep-particle->timeStamp);
    
    /* Check for crossing of cell borders: */
    queueCellChanges(particle,newPosition,timeStep,collisionQueue);
    
    /* Check for collision with any other particle: */
    GridCell* baseCell=particle->cell;
    for (int i=0;i<numNeighbors;++i)
        if (cellChangeMasks[i]&(1<<cellChangeDirection))
        {
            GridCell* cell=baseCell+neighborOffsets[i];
            queueCollisionsInCell(cell,particle,timeStep,true,0,collisionQueue);
        }
}

template <class ScalarT, int dimN>
inline
CollisionBox<ScalarT, dimN>::CollisionBox(
    const typename CollisionBox<ScalarT, dimN>::Box& sBoundaries,
    typename CollisionBox<ScalarT, dimN>::Scalar sParticleRadius,
    typename CollisionBox<ScalarT, dimN>::Scalar sSphereRadius)
    :boundaries(sBoundaries),
     numNeighbors(0),neighborOffsets(0),cellChangeMasks(0),
     particleRadius(sParticleRadius),particleRadius2(Math::sqr(particleRadius)),
     attenuation(1),
     numParticles(0),
     spherePosition(Point::origin),
     sphereVelocity(Vector::zero),
     sphereRadius(sSphereRadius),sphereRadius2(Math::sqr(sphereRadius)),
     sphereTimeStamp(0),
     latentForce(0),
     boxFriction(0),
     intraParticleGravitation(false)
{
    /* Calculate optimal number of cells and cell sizes: */
    Index numOuterCells;
    for (int i=0;i<dimension;++i)
    {
        numCells[i]=int(Math::floor(boundaries.getSize(i)/(particleRadius*Scalar(2))));
        cellSize[i]=boundaries.getSize(i)/Scalar(numCells[i]);
        numOuterCells[i]=numCells[i]+2; // Create a layer of "ghost cells" in all directions
    }
    
    /* Create the cell array: */
    cells.resize(numOuterCells);
    for (Index index(0);index[0]<numOuterCells[0];cells.preInc(index))
    {
        /* Initialize the cell: */
        Point min,max;
        for (int i=0;i<dimension;++i)
        {
            min[i]=boundaries.min[i]+cellSize[i]*Scalar(index[i]-1);
            max[i]=boundaries.min[i]+cellSize[i]*Scalar(index[i]-0);
        }
        cells(index).boundaries=Box(min,max);
        cells(index).particlesHead=0;
        cells(index).particlesTail=0;
    }
    
    /* Initialize the direct neighbor offsets: */
    for (int i=0;i<dimension;++i)
    {
        directNeighborOffsets[2*i+0]=-cells.getIncrement(i);
        directNeighborOffsets[2*i+1]=cells.getIncrement(i);
    }
    
    /* Initialize the neighbor array: */
    numNeighbors=1;
    for (int i=0;i<dimension;++i)
        numNeighbors*=3;
    neighborOffsets=new ssize_t[numNeighbors];
    cellChangeMasks=new int[numNeighbors];
    Index minBound(-1);
    Index maxBound(2);
    int neighborIndex=0;
    for (Index index=minBound;index[0]<maxBound[0];index.preInc(minBound,maxBound),++neighborIndex)
    {
        neighborOffsets[neighborIndex]=0;
        cellChangeMasks[neighborIndex]=0x0;
        for (int i=0;i<dimension;++i)
        {
            neighborOffsets[neighborIndex]+=cells.getIncrement(i)*index[i];
            if (index[i]==-1)
                cellChangeMasks[neighborIndex]|=1<<(2*i+0);
            else if (index[i]==1)
                cellChangeMasks[neighborIndex]|=1<<(2*i+1);
        }
    }
    
    /* Position the spherical obstacle: */
    spherePosition[0]=boundaries.min[0]-sphereRadius-Scalar(10);
    for (int i=1;i<dimension;++i)
        spherePosition[i]=Math::div2(boundaries.min[i]+boundaries.max[i]);
    sphereVelocity[0]=Scalar(5);
}

template <class ScalarT, int dimN>
inline
CollisionBox<ScalarT, dimN>::~CollisionBox(
    void)
{
    delete[] neighborOffsets;
    delete[] cellChangeMasks;
}

template <class ScalarT, int dimN>
inline
void
CollisionBox<ScalarT, dimN>::setAttenuation(
    typename CollisionBox<ScalarT, dimN>::Scalar newAttenuation)
{
    attenuation=newAttenuation;
}

template <class ScalarT, int dimN>
inline
bool
CollisionBox<ScalarT, dimN>::addParticle(
    const typename CollisionBox<ScalarT, dimN>::Point& newPosition,
    const typename CollisionBox<ScalarT, dimN>::Vector& newVelocity)
{
    /* Find the cell containing the new particle: */
    Point newP=newPosition;
    Index cellIndex;
    for (int i=0;i<dimension;++i)
    {
        if (newP[i]<boundaries.min[i]+particleRadius)
            newP[i]=boundaries.min[i]+particleRadius;
        else if (newP[i]>boundaries.max[i]-particleRadius)
            newP[i]=boundaries.max[i]-particleRadius;
        cellIndex[i]=int(Math::floor((newP[i]-boundaries.min[i])/cellSize[i]))+1;
    }
    GridCell* cell=cells.getAddress(cellIndex);
    
    /* Check if there is room to add the new particle: */
    for (int i=0;i<numNeighbors;++i)
    {
        GridCell* neighborCell=cell+neighborOffsets[i];
        
        for (Particle* pPtr=neighborCell->particlesHead;pPtr!=0;pPtr=pPtr->cellSucc)
        {
            Scalar dist2=Geometry::sqrDist(pPtr->position,newP);
            if (dist2<=Scalar(2)*particleRadius2)
                return false; // Could not add the particle
        }
    }
    /* Add a new particle to the particle list: */
    particles.push_back(Particle());
    Particle& p=particles.back();
    
    /* Initialize the new particle: */
    p.position=newPosition;
    p.velocity=newVelocity;
    p.timeStamp=Scalar(0);
    cell->addParticle(&p);

    for (typename ParticleList::iterator i = particles.begin(); i != particles.end() && &(*i) != &p; ++i) {
        if (&(*i) != &p) {
            particlePairs.push_back(ParticlePair(&p, &(*i)));
        }
    }
    
    return true; // Particle succesfully added
}

template <class ScalarT, int dimN>
inline
void
CollisionBox<ScalarT, dimN>::moveSphere(
    const typename CollisionBox<ScalarT, dimN>::Point& newPosition,
    typename CollisionBox<ScalarT, dimN>::Scalar timeStep)
{
    /* Calculate the sphere's velocity for this time step: */
    sphereVelocity=(newPosition-spherePosition)/timeStep;
}

template <class ScalarT, int dimN>
inline
void
CollisionBox<ScalarT, dimN>::simulate(
    typename CollisionBox<ScalarT, dimN>::Scalar timeStep)
{
    /* Initialize the collision queue: */
    CollisionQueue collisionQueue(numParticles*dimension);
    for (typename ParticleList::iterator pIt=particles.begin();pIt!=particles.end();++pIt)
        queueCollisions(&(*pIt),timeStep,false,0,collisionQueue);
    
    /* Update all particles' positions and handle all collisions: */
    while (!collisionQueue.isEmpty())
    {
        /* Get the next collision from the queue: */
        CollisionEvent nc=collisionQueue.getSmallest();
        collisionQueue.removeSmallest();
        
        /* Handle the collision if it is still valid: */
        switch (nc.collisionType)
        {
            case CollisionEvent::CellChange:
                if (nc.particle1->timeStamp==nc.timeStamp1)
                {
                    /* Let the particle cross into the next grid cell: */
                    GridCell* cell=nc.particle1->cell;
                    cell->removeParticle(nc.particle1);
                    cell+=directNeighborOffsets[nc.cellChangeDirection];
                    cell->addParticle(nc.particle1);
                    
                    /* Re-calculate all the particle's collisions: */
                    queueCollisionsOnCellChange(nc.particle1,timeStep,nc.cellChangeDirection,collisionQueue);
                }
                break;
            
            case CollisionEvent::WallCollision:
                if (nc.particle1->timeStamp==nc.timeStamp1)
                {
                    /* Bounce the particle off the wall: */
                    nc.particle1->position+=nc.particle1->velocity*(nc.collisionTime-nc.timeStamp1);
                    nc.particle1->timeStamp=nc.collisionTime;
                    Vector dv=(Scalar(2)*(nc.wallNormal*nc.particle1->velocity))*nc.wallNormal;
                    nc.particle1->velocity-=dv;
                    
                    /* Re-calculate all the particle's collisions: */
                    queueCollisions(nc.particle1,timeStep,true,0,collisionQueue);
                }
                break;
            
            case CollisionEvent::SphereCollision:
                if (nc.particle1->timeStamp==nc.timeStamp1 && sphereTimeStamp==nc.timeStamp2)
                {
                    /* Bounce the two particles off each other: */
                    nc.particle1->position+=nc.particle1->velocity*(nc.collisionTime-nc.timeStamp1);
                    nc.particle1->timeStamp=nc.collisionTime;
                    Point sp=spherePosition+sphereVelocity*(nc.collisionTime-nc.timeStamp2);
                    Vector d=sp-nc.particle1->position;
                    Scalar dLen2=Geometry::sqr(d);
                    Vector v1=d*((nc.particle1->velocity*d)/dLen2);
                    Vector v2=d*((sphereVelocity*d)/dLen2);
                    nc.particle1->velocity+=Scalar(2)*(v2-v1);
                    
                    /* Re-calculate all collisions of the particle: */
                    queueCollisions(nc.particle1,timeStep,true,0,collisionQueue);
                }
                break;
            
            case CollisionEvent::ParticleCollision:
                if (nc.particle1->timeStamp==nc.timeStamp1 && nc.particle2->timeStamp==nc.timeStamp2)
                {
                    /* Bounce the two particles off each other: */
                    nc.particle1->position+=nc.particle1->velocity*(nc.collisionTime-nc.timeStamp1);
                    nc.particle1->timeStamp=nc.collisionTime;
                    nc.particle2->position+=nc.particle2->velocity*(nc.collisionTime-nc.timeStamp2);
                    nc.particle2->timeStamp=nc.collisionTime;
                    Vector d=nc.particle2->position-nc.particle1->position;
                    Scalar dLen2=Geometry::sqr(d);
                    Vector v1=d*((nc.particle1->velocity*d)/dLen2);
                    Vector v2=d*((nc.particle2->velocity*d)/dLen2);
                    Vector dv=v2-v1;
                    nc.particle1->velocity+=dv;
                    nc.particle2->velocity-=dv;
                    
                    /* Re-calculate all collisions of both particles: */
                    queueCollisions(nc.particle1,timeStep,true,nc.particle2,collisionQueue);
                    queueCollisions(nc.particle2,timeStep,true,nc.particle1,collisionQueue);
                }
                break;
        }
    }
    
    // Scale attenuation factor for this time step
    Scalar att = (attenuation != Scalar(1) ? Math::pow(attenuation, timeStep) : Scalar(1));
    
    /* Update all particles to the end of the timestep: */
    auto updateFn = [this, att, timeStep](Particle& p) {
        p.position += p.velocity * (timeStep - p.timeStamp);
        p.velocity *= att;
        p.timeStamp = Scalar(0);
    };
    particles.forEach(updateFn);

    /* Apply any passive or active forces acting on the particles */
    auto fn = [this](Particle& p) {
        p.velocity += latentForce;
        p.velocity -= p.velocity * boxFriction;
    };
    particles.forEach(fn);

    if (intraParticleGravitation) {
        auto particlePull = [this](ParticlePair& pair) {
            Particle& p1 = *pair.p1;
            Particle& p2 = *pair.p2;
            Vector d = p1.position - p2.position;
            Scalar dLen2 = Geometry::sqr(d);
            p1.velocity -= d.normalize()/dLen2;
            p2.velocity += d.normalize()/dLen2;
        };
        particlePairs.forEach(particlePull);
    }
    
    /* Update the collision sphere to the end of the time step: */
    spherePosition+=sphereVelocity*(timeStep-sphereTimeStamp);
    sphereTimeStamp=Scalar(0);
}
