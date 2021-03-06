/***********************************************************************
CollisionBox - Class to represent a rectangular box containing spheres
of a fixed radius interacting by fully elastic collisions.
Copyright (c) 2005 Oliver Kreylos
***********************************************************************/

#ifndef COLLISIONBOX_INCLUDED
#define COLLISIONBOX_INCLUDED

#include <Misc/Array.h>
#include <Misc/ChunkedArray.h>
#include <Misc/PriorityHeap.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Box.h>

#include <Extra/Debug.h>
#include <list>

template <class ScalarParam, int dimensionParam>
class CollisionBox
{
public:
    /* Embedded classes: */
    typedef ScalarParam Scalar; // Data type for scalars
    static const int dimension=dimensionParam; // Dimension of collision box
    typedef Geometry::Point<Scalar, dimensionParam> Point; // Data type for points
    typedef Geometry::Vector<Scalar, dimensionParam> Vector; // Data type for points
    typedef Geometry::ComponentArray<Scalar, dimensionParam> Size; // Data type for sizes
    typedef Geometry::Box<Scalar, dimensionParam> Box; // Data type for axis-aligned boxes
    
private:
    struct GridCell; // Forward declaration
    struct CollisionEvent; // Forward declaration
    struct ParticlePair; // Forward declaration
    
public:
    class Particle // Class for fixed-radius spherical particles
    {
        friend class CollisionBox;
        friend class GridCell;
        friend class CollisionEvent;
        
    private:
        /* Elements: */
        Point position; // Current position of particle in collision box coordinates
        GridCell* cell; // Pointer to grid cell currently containing the particle
        Vector velocity; // Current velocity of particle in collision box coordinate
        Scalar timeStamp; // Time stamp of particle in current simulation step
        Particle* cellPred; // Pointer to particle's predecessor in same grid cell
        Particle* cellSucc; // Pointer to particle's successor in same grid cell
        
        /* Methods: */
    public:
        const Point& getPosition(void) const // Returns the particle's position
        {
            return position;
        }
        const Vector& getVelocity(void) const // Returns the particle's velocity
        {
            return velocity;
        }
    };
    
    typedef Misc::ChunkedArray<Particle> ParticleList; // Data type for lists of particles
    
private:
    struct GridCell // Structure for grid cells containing particles
    {
    public:
        /* Elements: */
        Box boundaries; // Cell's bounding box (waste of space, really; should optimize this out)
        Particle* particlesHead; // Pointer to first particle in grid cell
        Particle* particlesTail; // Pointer to last particle in grid cell
        
        /* Constructors and destructors: */
        GridCell(void) // Creates uninitialized grid cell
        {
        }
        
        /* Methods: */
        void addParticle(Particle* newParticle)
        {
            /* Link the new particle to the end of the cell's particle list: */
            newParticle->cell=this;
            newParticle->cellPred=particlesTail;
            newParticle->cellSucc=0;
            if (particlesTail!=0)
                particlesTail->cellSucc=newParticle;
            else
                particlesHead=newParticle;
            particlesTail=newParticle;
        }

        void removeParticle(Particle* removeParticle)
        {
            /* Unlink the particle from the end of the cell's list: */
            removeParticle->cell=0;
            if (removeParticle->cellPred!=0)
                removeParticle->cellPred->cellSucc=removeParticle->cellSucc;
            else
                particlesHead=removeParticle->cellSucc;
            if (removeParticle->cellSucc!=0)
                removeParticle->cellSucc->cellPred=removeParticle->cellPred;
            else
                particlesTail=removeParticle->cellPred;
        }

        void unshiftParticle(Particle* newParticle)
        {
            /* Link the new particle to the beginning of the cell's particle list: */
            newParticle->cell=this;
            newParticle->cellPred=0;
            newParticle->cellSucc=particlesHead;
            particlesHead=newParticle;
        }

        void shiftParticle(Particle* removeParticle)
        {
            /* Unlink the particle from the beginning of the cell's list: */
            removeParticle->cell=0;
            if (removeParticle->cellPred!=0)
                removeParticle->cellPred->cellSucc=removeParticle->cellSucc;
            else
                particlesHead=removeParticle->cellSucc;
            if (removeParticle->cellSucc!=0)
                removeParticle->cellSucc->cellPred=removeParticle->cellPred;
        }
    };
    
    typedef Misc::Array<GridCell, dimensionParam> CellArray; // Data type for arrays of grid cells
    typedef typename CellArray::Index Index; // Data type for cell indices
    
    struct CollisionEvent // Structure to report potential collisions between a particle and a wall or two particles
    {
        /* Embedded classes: */
        enum CollisionType // Two kinds of collision: particle/wall and particle/particle, and one pseudo-collision
        {
            CellChange, WallCollision, SphereCollision, ParticleCollision
        };
        
    public:
        /* Elements: */
        CollisionType collisionType; // Type of this collision
        Scalar collisionTime; // Time at which this collision would occur
        Particle* particle1; // Pointer to first colliding particle
        Scalar timeStamp1; // Time stamp of first particle at time collision was detected
        int cellChangeDirection; // The index of the cell border crossed by the object
        Vector wallNormal; // Normal vector of wall involved in collision
        Particle* particle2; // Pointer to second colliding particle
        Scalar timeStamp2; // Time stamp of second particle at time collision was detected
        
        /* Constructors and destructors: */
        CollisionEvent(Scalar sCollisionTime, Particle* sParticle1,
                       int sCellChangeDirection)
            :collisionType(CellChange), collisionTime(sCollisionTime), 
             particle1(sParticle1), timeStamp1(particle1->timeStamp), 
             cellChangeDirection(sCellChangeDirection),
             wallNormal(0), particle2(NULL), timeStamp2(Scalar(0))
        {
        }
        CollisionEvent(Scalar sCollisionTime, Particle* sParticle1,
                       const Vector& sWallNormal)
            :collisionType(WallCollision), collisionTime(sCollisionTime), 
             particle1(sParticle1), timeStamp1(particle1->timeStamp), 
             wallNormal(sWallNormal),
             particle2(NULL), timeStamp2(Scalar(0))
        {
        }
        CollisionEvent(Scalar sCollisionTime, Particle* sParticle1, Scalar sphereTimeStamp)
            :collisionType(SphereCollision), collisionTime(sCollisionTime), 
             particle1(sParticle1), timeStamp1(particle1->timeStamp), 
             particle2(NULL), timeStamp2(sphereTimeStamp)
        {
        }
        CollisionEvent(Scalar sCollisionTime, Particle* sParticle1, Particle* sParticle2)
            :collisionType(ParticleCollision), collisionTime(sCollisionTime), 
             particle1(sParticle1), timeStamp1(particle1->timeStamp), 
             particle2(sParticle2), timeStamp2(particle2->timeStamp)
        {
        }
        
        /* Methods: */
        friend bool operator<=(const CollisionEvent& e1, const CollisionEvent& e2)
        {
            return e1.collisionTime<=e2.collisionTime;
        }
    };
    
    typedef Misc::PriorityHeap<CollisionEvent> CollisionQueue; // Data types for priority queues of collision events
    
    struct ParticlePair {
        ParticlePair(Particle* p, Particle* q)
            : p1(p), p2(q) { }
        Particle* p1;
        Particle* p2;
    };

    typedef Misc::ChunkedArray<ParticlePair> ParticlePairs;

    ParticlePairs particlePairs;

    /* Elements: */
    Box boundaries; // Bounding box of entire collision box
    Size cellSize; // Size of an individual cell
    int numCells[dimension]; // Number of interior cells
    CellArray cells; // Array of grid cells
    ssize_t directNeighborOffsets[dimension*2]; // Offsets between a cell and its direct neighbors
    int numNeighbors; // Number of direct neighbors of a cell (including the cell itself)
    ssize_t* neighborOffsets; // Pointer offsets between a cell and its neighbors
    int* cellChangeMasks; // Array of cell change direction masks for each neighbor
    Scalar particleRadius, particleRadius2; // Radius and squared radius of all particles
    Scalar attenuation; // Factor by how much particles slow down over the course of one time unit; ==1: no slowdown
    size_t numParticles; // Number of particles in the collision box
    ParticleList particles; // List of all particles in the collision box
    Point spherePosition; // Position of an additional spherical obstacle
    Vector sphereVelocity; // Velocity of spherical obstacle
    Scalar sphereRadius, sphereRadius2; // Radius and squared radius of spherical obstacle
    Scalar sphereTimeStamp; // Time stamp of spherical obstacle in current time step
    Vector latentForce; // Force (e.g. gravity) to apply to the particles at every step
    Scalar boxFriction; // Latent friction applied to all particles
    bool intraParticleGravitation; // Whether or not to simulate gravity between particles

    /* Private methods: */
    void queueCollisionsInCell(GridCell* cell, Particle* particle1, Scalar timeStep,
                               bool symmetric, Particle* otherParticle,
                               CollisionQueue& collisionQueue);
    void queueCellChanges(Particle* particle, const Point& newPosition,
                          Scalar timeStep, CollisionQueue& collisionQueue);
    void queueCollisions(Particle* particle1, Scalar timeStep, bool symmetric,
                         Particle* otherParticle, CollisionQueue& collisionQueue);
    void queueCollisionsOnCellChange(Particle* particle1, Scalar timeStep,
                                     int cellChangeDirection,
                                     CollisionQueue& collisionQueue);
    
    /* Constructors and destructors: */
public:
    CollisionBox(const Box& sBoundaries, Scalar sParticleRadius, Scalar sSphereRadius); // Creates box of given size, for particles of given radius
    ~CollisionBox(void); // Destroys collision box and all particles
    
    /* Methods: */
    const Box& getBoundaries(void) const { // Returns the collision box's boundaries
        return boundaries;
    }
    void setAttenuation(Scalar newAttenuation); // Sets new attenuation factor for particle velocities
    bool addParticle(const Point& newPosition, const Vector& newVelocity); // Adds a new particle to the collision box; returns false if particle could not be added due to overlap with existing particles
    void moveSphere(const Point& newPosition, Scalar timeStep); // Moves the spherical obstacle to the given position at the end of the next time step
    void simulate(Scalar timeStep); // Advances simulation time by given time step
    void setLatentForce(const Vector& force) {
        latentForce = force;
    }
    void setFriction(const Scalar& friction) {
        boxFriction = friction;
    }
    void setIntraParticleGravitation(bool enable) {
        intraParticleGravitation = enable;
    }
    const ParticleList& getParticles(void) const { // Returns the list of particles
        return particles;
    }
    const Point& getSphere(void) const { // Returns the collision sphere's current position
        return spherePosition;
    }
};

#ifndef COLLISIONBOX_IMPLEMENTATION
#include "CollisionBox.cpp"
#endif

#endif
