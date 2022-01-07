#ifndef ACTORWORLD_H_
#define ACTORWORLD_H_

#include "GameWorld.h"
#include <string>
#include <list>

class Actor;
class Socrates;
class Projectile;
class Bacteria;

class ActorWorld : public GameWorld
{
public:
    ActorWorld(std::string assetPath);
    ~ActorWorld();
        // Core Functions
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
        // General Auxiliary Functions
    void decPitCount();
    void decBacteriaCount();
    bool finishedLevel() const;
    
        // Socrates Auxiliary Functions
    bool socratesOverlap(Actor* actor);
    
        // Bacteria Auxiliary Functions
    bool bacteriaSocratesOverlap(Bacteria* bacteria);
    bool bacteriaMovementBlocked(const double& attemptX, const double& attemptY) const;
    bool nearSocrates(Bacteria* aggSal, double& socratesX, double& socratesY, const double& dist);
    bool canEatFood(Bacteria* bacteria);
    bool findFood(Bacteria* bacteria, double& foodX, double& foodY);
    void dropFood(double x, double y);
    
    
        // Bacteria Spawning Functions
    void spawnRegSal(double startX, double startY);
    void spawnAggSal(double startX, double startY);
    void spawnEColi(double startX, double startY);
    
        // Flame Auxiliary Functions
    void throwFlames();
    void flameDamage(Projectile* flame);
    
        // Spray Auxiliary Functions
    void sprayDisinfectant();
    void sprayDamage(Projectile* spray);
    
        // Health Goodie Auxiliary Functions
    void healSocrates();
    
        // Flamethrower Goodie Auxiliary Functions
    void rechargeFlamethrower();
    
        // Fungus Auxiliary Functions
    void fungusHurtSocrates();

private:
    Socrates* socrates;
    std::list<Actor*> actors;
    int m_bacteria;
    int m_pits;
    
        // Supporting Functions
    void generateRandPos(double& x, double& y);
    void generateRandPosOnBorder(double& x, double& y);
    void removeDeadActors();
    void addGoodiesOrFungi();
    void updateStatusText();
};

#endif // ACTORWORLD_H_
