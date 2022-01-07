#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Forward declaration of ActorWorld
class ActorWorld;

///////////////////////////////////////////
// Actor Definition
///////////////////////////////////////////
class Actor : public GraphObject {
public:
    Actor(ActorWorld* world, int imageID, double startX, double startY, Direction startDirection, int depth = 0);
    virtual ~Actor() {}
        // All actors must doSomething specific to their type each tick
    virtual void doSomething() = 0;
    
        // Actor identifiers - all return false in base class
    virtual bool isDamageable() const;
    virtual bool isCharacter() const;
    virtual bool isBacteria() const;
    virtual bool isGoodie() const;
    virtual bool isBlocker() const;
    virtual bool isProjectile() const;
    virtual bool isEdible() const;
    virtual bool isBacteriaSpawner() const;
    
        // Accessors
    ActorWorld* getWorld() const;
    bool isAlive() const;
    
        // Mutators
    virtual void setDead();
    
        // Auxiliary functions
    bool overlaps(Actor* a1, Actor* a2) const;
    bool movementOverlap(double attemptX, double attemptY, Actor* blocker);
    int distance(Actor* a1, Actor* a2) const;
    int distance(double x1, double y1, Actor* a) const;
    int distanceFromCentre(double x, double y);
    
private:
    ActorWorld* m_world;
    bool m_alive;
};
    // Damageable, Projectile, Food and Pit inherit from this

/////////////////////////////////////////////////////
// Damageable Definition
/////////////////////////////////////////////////////
    // It can take damage and be killed this way (inheriting from Actor)
class Damageable : public Actor {
public:
    Damageable(ActorWorld* world, int imageID, double startX, double startY, Direction startDir = right, int depth = 1);
    virtual ~Damageable() {}
        // Identifier
    virtual bool isDamageable() const;
};
    // Charater, Goodies, and Dirt inherit from this

/////////////////////////////////////////////////////
// Character Definition
/////////////////////////////////////////////////////
    // It has health, so can't be killed instantly (inheriting from Damageable)
class Character : public Damageable {
public:
    Character(ActorWorld* world, int imageID, double startX, double startY, int startHP, Direction startDir = right);
    virtual ~Character() {}
        // Identifier
    virtual bool isCharacter() const;
    
        // Accessors
    int getHealth() const;
    
        // Mutators
    virtual void damageCharacter(int dmg);
    
protected:
        // Protected Mutator
    void setHealth(int hp);
private:
    int m_health;
};
    // Socrates and Bacteria inherit from this

///////////////////////////////////////////////
// Socrates Definition
///////////////////////////////////////////////
    // Socrates is the player (inheriting from Character)
class Socrates : public Character {
public:
    Socrates(ActorWorld* world);
        // Socrates must do something every tick
    virtual void doSomething();
    
        // Accessors
    int spraysRemaining() const;
    int flamesRemaining() const;
    
        // Mutators
    virtual void damageCharacter(int dmg);
    void restoreHealthToFull();
    void chargeFlamethrower();
    
        // Auxiliary Functions
    void sprayDisinfectant();
    void throwFlames();
    
private:
    int m_sprayCharges;
    int m_flameCharges;
    bool m_sprayCanRecharge;
};

///////////////////////////////////////////////
// Bacteria Definition
///////////////////////////////////////////////
    // Bacteria are the enemies and can't be killed instantly, i.e. has health (inheriting from Character)
class Bacteria : public Character {
public:
    Bacteria(ActorWorld* world, int imageID, double startX, double startY, int startHP, int damageToken);
    virtual ~Bacteria() {}
        // All Bacteria start the tick by doing the same thing
    virtual void doSomething();
        // All Bacteria then take specific actions in accordance with their type
    virtual void specificBacteriaAction() = 0;
    
        // Identifer
    virtual bool isBacteria() const;
    
        // Accessors
    int getDamageToken() const;
    
        // Mutators
    virtual void setDead();
    
protected:
        // Protected Accessors
    int getFoodEaten() const;
    int getMovementPlanDist() const;
    
        // Protected Mutators
    void incFoodEaten();
    void resetFoodEaten();
    void decMovementPlanDist();
    void resetMovementPlanDist();
    
        // Protected Auxiliaries
    virtual void divide(double& newX, double& newY) = 0;    // each type of Bacteria divides into their same type, so each type of Bacteria must have their own implementation (which calls this function)
    virtual bool attemptMove(const int& attemptedDist);
    Direction findDirectionTo(const double& targetX, const double& targetY);
    void divideOrEat();
    void attemptToDamageSocrates();

    
private:
    int m_damageToken;
    int m_movementPlanDist;
    int m_foodEaten;
    
        // Private Auxiliary Functions
    void getDivideCoords(double& newX, double& newY);
    bool willMoveOutsideOfPetri(double attemptX, double attemptY);
};
    // Salmonella and EColi inherit from this

///////////////////////////////////////////////////
// Salmonella Definition
///////////////////////////////////////////////////
    // There are two types of Salmonella that have similar behaviour and properties (inheriting from Bacteria)
class Salmonella : public Bacteria {
public:
    Salmonella(ActorWorld* world, double startX, double startY, int startHP, int damageToken);
    virtual ~Salmonella() {}
        // Salmonella take a specific action every tick
    virtual void specificBacteriaAction();
    
        // Mutators
    virtual void damageCharacter(int dmg);
    
protected:
        // Protected Auxiliary Functions
    virtual bool aggressivePathfind() = 0;      // one type of Salmonella is not aggressive, the other is
    void salmonellaMovementAttempt();
    void foodPathfind();
    void selectNewDir();
};
    // RegularSalmonella, AggressiveSalmonella inherit from this

///////////////////////////////////////////////////////////////////////
// RegularSalmonella Definition
///////////////////////////////////////////////////////////////////////
class RegularSalmonella : public Salmonella {
public:
    RegularSalmonella(ActorWorld* world, double startX, double startY);
    
protected:
        // Protected Auxiliary Functions
    virtual void divide(double& spawnX, double& spawnY);
    virtual bool aggressivePathfind();
};

///////////////////////////////////////////////////////////////////////////
// AggressiveSalmonella Definition
///////////////////////////////////////////////////////////////////////////
class AggressiveSalmonella : public Salmonella {
public:
    AggressiveSalmonella(ActorWorld* world, double startX, double startY);
    
protected:
        // Protected Auxiliary Functions
    virtual void divide(double& spawnX, double& spawnY);
    virtual bool aggressivePathfind();
};

///////////////////////////////////////////
// EColi Definition
///////////////////////////////////////////
class EColi : public Bacteria {
public:
    EColi(ActorWorld* world, double startX, double startY);
        // EColi take a specific action every tick
    virtual void specificBacteriaAction();
    
        // Mutators
        virtual void damageCharacter(int dmg);
    
protected:
        // Protected Auxiliary Functions
    virtual void divide(double& newX, double& newY);
};

/////////////////////////////////////////////////////////
// Goodie Definition
/////////////////////////////////////////////////////////
    // Provides a buff/debuff to Socrates (inheriting from Damageable)
class Goodie : public Damageable {
public:
    Goodie(ActorWorld* world, int imageID, double startX, double startY, int lifetime, int scoreVal);
    virtual ~Goodie() {}
        // All Goodies do very similar things
    virtual void doSomething();
        // All Goodies perform an action specific to their type
    virtual void specificGoodieAction() = 0;
    
        // Identifier
    virtual bool isGoodie() const;
    
protected:
        // Protected Mutators
    void decRemainingTime();
    
        // Protected Accessors
    int getScoreValue() const;
    
private:
    int m_timeRemaining;
    int m_scoreValue;
};
    // HealthGoodie, FlameGoodie, LifeGoodie and Fugus inherit from this

///////////////////////////////////////////////////////////////////////
// HealthGoodie Definition
///////////////////////////////////////////////////////////////////////
    // Restores health and gives score (inheriting from Goodie)
class HealthGoodie : public Goodie {
public:
    HealthGoodie(ActorWorld* world, double startX, double startY);
    virtual void specificGoodieAction();
};

//////////////////////////////////////////////////////////////////////////
// FlameGoodie Definition
//////////////////////////////////////////////////////////////////////////
    // Recharges flames and gives score (inheriting from Goodie)
class FlameGoodie : public Goodie {
public:
    FlameGoodie(ActorWorld* world, double startX, double startY);
    virtual void specificGoodieAction();
};

//////////////////////////////////////////////////////////////////////////////
// LifeGoodie Definition
//////////////////////////////////////////////////////////////////////////////
    // Adds a life and gives score (inheriting from Goodie)
class LifeGoodie : public Goodie {
public:
    LifeGoodie(ActorWorld* world, double startX, double startY);
    virtual void specificGoodieAction();
};

/////////////////////////////////////////////////////////
// Fungus Definition
/////////////////////////////////////////////////////////
    // Causes damage and reduces score (inheriting from Goodie)
class Fungus : public Goodie {
public:
    Fungus(ActorWorld* world, double startX, double startY);
    virtual void specificGoodieAction();
};

///////////////////////////////////////////
// Dirt Definition
///////////////////////////////////////////
    // Doesn't have health (instantly killable) and doesn't provide a buff/debuff (inheriting from Damageable)
class Dirt : public Damageable {
public:
    Dirt(ActorWorld* world, double startX, double startY);
        // Dirt must do something every tick
    virtual void doSomething();
        // Identifier
    virtual bool isBlocker() const;
};

/////////////////////////////////////////////////////
// Projectile Definition
/////////////////////////////////////////////////////
    // Is something Socrates shoots to cause damage (inheriting from Actor)
class Projectile : public Actor {
public:
    Projectile(ActorWorld* world, int imageID, double startX, double startY, Direction startDir, int maxTravelDist, int damageToken);
    virtual ~Projectile() {}
        // All Projectiles start the tick by doing the same thing
    virtual void doSomething();
        // All Projectiles take an action specific to their type each tick
    virtual void specificProjectileAction() = 0;
    
        // Identifier
    virtual bool isProjectile() const;
    
        // Accessors
    int getDamageToken() const;
    
protected:
        // Protected Accessors
    int distanceTravelled() const;
    
        // Protected Mutators
    void travelDistance(int dist);
    
private:
    int m_maxTravelDistance;
    int m_distanceTravelled;
    int m_damageToken;
};
    // Flame and Spray inherit from this

/////////////////////////////////////////////////////
// Flame Definition
/////////////////////////////////////////////////////
    // Is a flame projectile (inheriting from Projectile)
class Flame : public Projectile {
public:
    Flame(ActorWorld* world, double startX, double startY, Direction startDir);
    virtual void specificProjectileAction();
};

/////////////////////////////////////////////////////
// Spray Definition
/////////////////////////////////////////////////////
    // Is a spray projectile (inheriting from Projectile)
class Spray : public Projectile {
public:
    Spray(ActorWorld* world, double startX, double startY, Direction startDir);
    virtual void specificProjectileAction();
};

///////////////////////////////////////////
// Food Definition
///////////////////////////////////////////
    // Is some food (inheriting from Actor)
class Food : public Actor {
public:
    Food(ActorWorld* world, double startX, double startY);
        // Food must do something every tick
    virtual void doSomething();
        // Identifier
    virtual bool isEdible() const;
};

/////////////////////////////////////////////////////////
// Pit Definition
/////////////////////////////////////////////////////////
    // Is a bacterial spawning pit (inheriting from Actor)
class Pit : public Actor {
public:
    Pit(ActorWorld* world, double startX, double startY);
        // Pits must do something every tick
    virtual void doSomething();
    
        // Identifier
    virtual bool isBacteriaSpawner() const;
        // Mutator
    virtual void setDead();
    
private:
    int m_regSalLeft;
    int m_aggSalLeft;
    int m_eColiLeft;
    
        // Private Auxiliary Functions
    bool isPitEmpty() const;
    int spawnWhichBacteria() const;
};

#endif // ACTOR_H_
