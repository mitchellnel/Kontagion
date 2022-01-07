#include "Actor.h"
#include "ActorWorld.h"
#include <cmath>
#include <list>

using namespace std;

///////////////////////////////////////////////////
// Actor Implementation
///////////////////////////////////////////////////
Actor::Actor(ActorWorld* world, int imageID, double startX, double startY, Direction startDir, int depth) : GraphObject(imageID, startX, startY, startDir, depth), m_world(world), m_alive(true)
{}

    // Identifiers
bool Actor::isDamageable() const {
    return false;
}

bool Actor::isCharacter() const {
    return false;
}

bool Actor::isBacteria() const {
    return false;
}

bool Actor::isGoodie() const {
    return false;
}

bool Actor::isBlocker() const {
    return false;
}

bool Actor::isProjectile() const {
    return false;
}

bool Actor::isEdible() const {
    return false;
}

bool Actor::isBacteriaSpawner() const {
    return false;
}

    // Accessors
ActorWorld* Actor::getWorld() const {
    return m_world;
}

bool Actor::isAlive() const {
    return m_alive;
}

    // Mutators
void Actor::setDead() {
    m_alive = false;
}


    // Auxiliary Functions
bool Actor::overlaps(Actor* a1, Actor* a2) const {
    return distance(a1, a2) < SPRITE_WIDTH;
}

bool Actor::movementOverlap(double attemptX, double attemptY, Actor* blocker) {
    return distance(attemptX, attemptY, blocker) <= SPRITE_WIDTH / 2;
}

int Actor::distance(Actor* a1, Actor* a2) const {
    double x1 = a1->getX(), y1 = a1->getY();
    double x2 = a2->getX(), y2 = a2->getY();
    
    // returns Euclidean distance between Actors a1 and a2
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

int Actor::distance(double x1, double y1, Actor* a) const {
    double x2 = a->getX(), y2 = a->getY();
    
    // returns Euclidean distance between point (x1, y1) and Actor a
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

int Actor::distanceFromCentre(double x, double y) {
    x = abs(x - VIEW_WIDTH / 2);
    y = abs(y - VIEW_HEIGHT / 2);
    
    // returns Euclidean distance between point (x ,y) and centre (VIEW_WIDTH / 2 , VIEW_HEIGHT / 2)
    return sqrt(pow(x, 2) + pow(y, 2));
}

/////////////////////////////////////////////////////////////
// Damageable Implementation
/////////////////////////////////////////////////////////////
Damageable::Damageable(ActorWorld* world, int imageID, double startX, double startY, Direction startDir, int depth) : Actor(world, imageID, startX, startY, startDir, depth)
{}

    // Identifier
bool Damageable::isDamageable() const {
    return true;
}

/////////////////////////////////////////////////////////////
// Character Implementation
/////////////////////////////////////////////////////////////
Character::Character(ActorWorld* world, int imageID, double startX, double startY, int startHP, Direction startDir) : Damageable(world, imageID, startX, startY, startDir, 0), m_health(startHP)
{}

    // Identifier
bool Character::isCharacter() const {
    return true;
}

    // Accessors
int Character::getHealth() const {
    return m_health;
}

    // Mutators
void Character::damageCharacter(int dmg) {
    if (dmg < 0)
        return;
    
    m_health -= dmg;
    
    if (m_health <= 0)
        setDead();
}

    // Protected Mutators
void Character::setHealth(int hp) {
    m_health = hp;
}

/////////////////////////////////////////////////////////
// Socrates Implementation
/////////////////////////////////////////////////////////
Socrates::Socrates(ActorWorld* world) : Character(world, IID_PLAYER, 0, VIEW_HEIGHT/2, 100), m_sprayCharges(20), m_flameCharges(5)
{}

    // Socrates will do something every tick - move, spray/flame, or recharge his sprays
void Socrates::doSomething() {
    if (!isAlive())
        return;
    
    int kp;
    if (getWorld()->getKey(kp)) {
        switch (kp) {
            case KEY_PRESS_LEFT: {
                moveAngle(getDirection(), VIEW_WIDTH/2);        // move to centre
                setDirection(getDirection()+5);                 // adjust positional angle (which is relative to centre)
                moveAngle(getDirection(), -VIEW_WIDTH/2);       // move back to border of game area
                m_sprayCanRecharge = true;
                break;
            }
                
            case KEY_PRESS_RIGHT:
                moveAngle(getDirection(), VIEW_WIDTH/2);
                setDirection(getDirection()-5);
                moveAngle(getDirection(), -VIEW_WIDTH/2);
                m_sprayCanRecharge = true;

                break;
                
            case KEY_PRESS_SPACE:
                if (m_sprayCharges >= 1) {
                    sprayDisinfectant();
                    m_sprayCanRecharge = false;
                }
                break;
                
            case KEY_PRESS_ENTER:
                if (m_flameCharges >= 1)
                    throwFlames();
                m_sprayCanRecharge = true;
                break;
        }
    } else {
        if (m_sprayCanRecharge && m_sprayCharges < 20)
            m_sprayCharges++;
        
        m_sprayCanRecharge = true;
    }
}

    // Accessors
int Socrates::spraysRemaining() const {
    return m_sprayCharges;
}

int Socrates::flamesRemaining() const {
    return m_flameCharges;
}

    // Mutators
void Socrates::damageCharacter(int dmg) {
    Character::damageCharacter(dmg);
    
    if (!isAlive())
        getWorld()->playSound(SOUND_PLAYER_DIE);
    else
        getWorld()->playSound(SOUND_PLAYER_HURT);
}

void Socrates::restoreHealthToFull() {
    setHealth(100);
}

void Socrates::chargeFlamethrower() {
    m_flameCharges += 5;
}

    // Auxiliary Functions
void Socrates::sprayDisinfectant() {
    getWorld()->sprayDisinfectant();

    m_sprayCharges--;
    getWorld()->playSound(SOUND_PLAYER_SPRAY);
}

void Socrates::throwFlames() {
    getWorld()->throwFlames();
    
    m_flameCharges--;
    getWorld()->playSound(SOUND_PLAYER_FIRE);
}

//////////////////////////////////////////////////////////
// Bacteria Implementation
//////////////////////////////////////////////////////////
Bacteria::Bacteria(ActorWorld* world, int imageID, double startX, double startY, int startHP, int damageToken) : Character(world, imageID, startX, startY, startHP, up), m_damageToken(damageToken), m_movementPlanDist(0), m_foodEaten(0)
{}

    // All Bacteria start the tick by checking if they're alive - they each take their own series of actions after that
void Bacteria::doSomething() {
    if (!isAlive())
        return;
    
    specificBacteriaAction();
}

    // Identifier
bool Bacteria::isBacteria() const {
    return true;
}
    
    // Accessors
int Bacteria::getDamageToken() const {
    return m_damageToken;
}

    // Mutators
void Bacteria::setDead() {
    Actor::setDead();
    
    // Update the ActorWorld
    getWorld()->increaseScore(100);
    getWorld()->dropFood(getX(), getY());
    getWorld()->decBacteriaCount();
}

    // Protected Accessors
int Bacteria::getFoodEaten() const {
    return m_foodEaten;
}

int Bacteria::getMovementPlanDist() const {
    return m_movementPlanDist;
}

    // Protected Mutators
void Bacteria::incFoodEaten() {
    m_foodEaten++;
}

void Bacteria::resetFoodEaten() {
    m_foodEaten = 0;
}

void Bacteria::decMovementPlanDist() {
    m_movementPlanDist--;
}

void Bacteria::resetMovementPlanDist() {
    m_movementPlanDist = 10;
}

    // Protected Auxiliary Functions
void Bacteria::divide(double& spawnX, double& spawnY) {
    getDivideCoords(spawnX, spawnY);
    resetFoodEaten();
}

bool Bacteria::attemptMove(const int& attemptedDist) {
    // Can the Bacteria move forward attemptedDist pixels in its current direction?
    double attemptX, attemptY;
    getPositionInThisDirection(getDirection(), attemptedDist, attemptX, attemptY);
    
    if (getWorld()->bacteriaMovementBlocked(attemptX, attemptY) || willMoveOutsideOfPetri(attemptX, attemptY))
        return false;
    else
        return true;
}

Direction Bacteria::findDirectionTo(const double& targetX, const double& targetY) {
    if (targetX == getX()) {          // Target is on the same horizontal
        if (targetY > getY())
            return up;
        else
            return down;
    } else if (targetY == getY()) {   // Target is on the same vertical
        if (targetX > getX())
            return right;
        else
            return left;
    } else {
        double theta = 0;   // theta in radians
        if (targetX < getX() && targetY > getY()) {  // unit circle - quadrant 2
            theta = M_PI - atan2(abs(targetY - getY()), abs(targetX - getX()));
        } else if (targetX < getX() && targetY < getY()) {  // unit circle - quadrant 3
            theta = M_PI + atan2(abs(targetY - getY()), abs(targetX - getX()));
        } else if (targetX > getX() && targetY < getY()) {  // unit circle - quadrant 4
            theta = 2 * M_PI - atan2(abs(targetY - getY()), abs(targetX - getX()));
        } else {                                        // unit circle - quadrant 1
            theta = atan2(abs(targetY - getY()), abs(targetX - getX()));
        }
        return theta * (180 / M_PI);    // convert to degrees and return
    }
}

void Bacteria::divideOrEat() {
    if (getFoodEaten() == 3) {
        double spawnX, spawnY;
        divide(spawnX, spawnY);
    } else {
        if (getWorld()->canEatFood(this))
            incFoodEaten();
    }
}

void Bacteria::attemptToDamageSocrates() {
    if (!getWorld()->bacteriaSocratesOverlap(this))
        // we are not overlapping with Socrates --> so try to divide or eat
        divideOrEat();
    // else Socrates will take damage in the call to bacteriaSocratesOverlap()
}

    // Private Auxiliary Functions
void Bacteria::getDivideCoords(double& newX, double& newY) {
    if (getX() < VIEW_WIDTH / 2)
        newX = getX() + SPRITE_WIDTH / 2;
    else if (getX() > VIEW_WIDTH / 2)
        newX = getX() - SPRITE_WIDTH / 2;
    else
        newX = getX();
    
    if (getY() < VIEW_HEIGHT / 2)
        newY = getY() + SPRITE_WIDTH / 2;
    else if (getY() > VIEW_HEIGHT / 2)
        newY = getY() - SPRITE_WIDTH / 2;
    else
        newY = getY();
}

bool Bacteria::willMoveOutsideOfPetri(double attemptX, double attemptY) {
    return distanceFromCentre(attemptX, attemptY) >= VIEW_RADIUS;
}

/////////////////////////////////////////////////////////////
// Salmonella Implementation
/////////////////////////////////////////////////////////////
Salmonella::Salmonella(ActorWorld* world, double startX, double startY, int startHP, int damageToken) : Bacteria(world, IID_SALMONELLA, startX, startY, startHP, damageToken)
{}

void Salmonella::specificBacteriaAction() {
    if (aggressivePathfind())
        return;
    
    attemptToDamageSocrates();
    
    if (getMovementPlanDist() > 0) {
        salmonellaMovementAttempt();
        return;
    }
    
    foodPathfind();
}

void Salmonella::damageCharacter(int dmg) {
    Character::damageCharacter(dmg);
    
    if (!isAlive()) {
        getWorld()->playSound(SOUND_SALMONELLA_DIE);
    } else {
        getWorld()->playSound(SOUND_SALMONELLA_HURT);
    }
}

void Salmonella::salmonellaMovementAttempt() {
    decMovementPlanDist();
    
    if (attemptMove(3)) {
        moveForward(3);
    } else {
        selectNewDir();
    }
}

void Salmonella::foodPathfind() {
    double foodX, foodY;
    if (getWorld()->findFood(this, foodX, foodY)) {
        // find the direction the Food is in, and set to that direction
        setDirection(findDirectionTo(foodX, foodY));
        
        salmonellaMovementAttempt();
    } else {
        selectNewDir();
    }
}

void Salmonella::selectNewDir() {
    Direction newDir = randInt(0, 359);
    setDirection(newDir);
    resetMovementPlanDist();
}

//////////////////////////////////////////////////////////////////////////////
// RegularSalmonella Implementation
//////////////////////////////////////////////////////////////////////////////
RegularSalmonella::RegularSalmonella(ActorWorld* world, double startX, double startY) : Salmonella(world, startX, startY, 4, 1)
{}

    // Protected Auxiliary Functions
void RegularSalmonella::divide(double& spawnX, double& spawnY) {
    Bacteria::divide(spawnX, spawnY);
    getWorld()->spawnRegSal(spawnX, spawnY);
}

bool RegularSalmonella::aggressivePathfind() {
    // RegularSalmonella does not aggressively find paths to Socrates, just return false to indicate no aggression undertaken
    return false;
}

////////////////////////////////////////////////////////////////////////////////////
// AggressiveSalmonella Implementation
////////////////////////////////////////////////////////////////////////////////////
AggressiveSalmonella::AggressiveSalmonella(ActorWorld* world, double startX, double startY) : Salmonella(world, startX, startY, 10, 2)
{}

    // Protected Auxiliary Functions
void AggressiveSalmonella::divide(double& spawnX, double& spawnY) {
    Bacteria::divide(spawnX, spawnY);
    getWorld()->spawnAggSal(spawnX, spawnY);
}

bool AggressiveSalmonella::aggressivePathfind() {
    // AggressiveSalmonella will aggressively seek out Socrates if they're within 72 pixels of him
    double socratesX, socratesY;
    if (getWorld()->nearSocrates(this, socratesX, socratesY, 72)) {
        setDirection(findDirectionTo(socratesX, socratesY));
        
        if (attemptMove(3))
            moveForward(3);
        
        attemptToDamageSocrates();
        return true;    // there was aggression this tick
    }
    return false;       // there wasn't aggression this tick
}

//////////////////////////////////////////////////////////
// EColi Implementation
//////////////////////////////////////////////////////////
EColi::EColi(ActorWorld* world, double startX, double startY) : Bacteria(world, IID_ECOLI, startX, startY, 10, 4)
{}

void EColi::specificBacteriaAction() {
    attemptToDamageSocrates();
    
        // EColi will seek out Socrates across the entire Petri dish (256 pixels)
    double socratesX, socratesY;
    if (getWorld()->nearSocrates(this, socratesX, socratesY, 256)) {
        setDirection(findDirectionTo(socratesX, socratesY));
        for (int i = 0; i < 10; i++) {
            if (attemptMove(2)) {
                moveForward(2);
                return;
            } else {
                setDirection(getDirection()+10);
            }
        }
    }
}

    // Mutators
void EColi::damageCharacter(int dmg) {
    Character::damageCharacter(dmg);
    
    if (!isAlive()) {
        getWorld()->playSound(SOUND_ECOLI_DIE);
    } else {
        getWorld()->playSound(SOUND_ECOLI_HURT);
    }
}

    // Protected Auxiliary Functions
void EColi::divide(double& spawnX, double& spawnY) {
    Bacteria::divide(spawnX, spawnY);
    getWorld()->spawnEColi(spawnX, spawnY);
}

/////////////////////////////////////////////////////////
// Goodie Implementation
/////////////////////////////////////////////////////////
Goodie::Goodie(ActorWorld* world, int imageID, double startX, double startY, int lifetime, int scoreVal) : Damageable(world, imageID, startX, startY), m_timeRemaining(lifetime), m_scoreValue(scoreVal)
{}
    // All Goodies start the tick by checking if they're alive
void Goodie::doSomething() {
    if (!isAlive())
        return;
    
    if (getWorld()->socratesOverlap(this)) {
        specificGoodieAction();
        setDead();
    } else {
        decRemainingTime();
    }
}

void Goodie::specificGoodieAction() {
    getWorld()->playSound(SOUND_GOT_GOODIE);
    getWorld()->increaseScore(getScoreValue());
}

    // Identifier
bool Goodie::isGoodie() const {
    return true;
}

    // Mutators
void Goodie::decRemainingTime() {
    m_timeRemaining--;
    
    if (m_timeRemaining <= 0)
        setDead();
}
    
    // Protected Accessors
int Goodie::getScoreValue() const {
    return m_scoreValue;
}

///////////////////////////////////////////////////////////////////////
// HealthGoodie Implementation
///////////////////////////////////////////////////////////////////////
HealthGoodie::HealthGoodie(ActorWorld* world, double startX, double startY)
    : Goodie(world, IID_RESTORE_HEALTH_GOODIE, startX, startY, max(randInt(0, 300 - 10 * world->getLevel() -1), 50), 250)
{}

void HealthGoodie::specificGoodieAction() {
    Goodie::specificGoodieAction();
    getWorld()->healSocrates();
}

/////////////////////////////////////////////////////////////////////////////////////
// FlameGoodie Implementation
/////////////////////////////////////////////////////////////////////////////////////
FlameGoodie::FlameGoodie(ActorWorld* world, double startX, double startY)
    : Goodie(world, IID_FLAME_THROWER_GOODIE, startX, startY, max(randInt(0, 300 - 10 * world->getLevel() - 1), 50), 300)
{}

void FlameGoodie::specificGoodieAction() {
    Goodie::specificGoodieAction();
    getWorld()->rechargeFlamethrower();
}

//////////////////////////////////////////////////////////////////////////////
// LifeGoodie Implementation
//////////////////////////////////////////////////////////////////////////////
LifeGoodie::LifeGoodie(ActorWorld* world, double startX, double startY)
    : Goodie(world, IID_EXTRA_LIFE_GOODIE, startX, startY, max(randInt(0, 300 - 10 * world->getLevel() - 1), 50), 500)
{}

void LifeGoodie::specificGoodieAction() {
    Goodie::specificGoodieAction();
    getWorld()->incLives();
}

/////////////////////////////////////////////////////////
// Fungus Implementation
/////////////////////////////////////////////////////////
Fungus::Fungus(ActorWorld* world, double startX, double startY)
    : Goodie(world, IID_FUNGUS, startX, startY, max(randInt(0, 300 - 10 * world->getLevel() - 1), 50), -50)
{}

void Fungus::specificGoodieAction() {
    getWorld()->increaseScore(getScoreValue());
    getWorld()->fungusHurtSocrates();
}

/////////////////////////////////////////////////////////
// Dirt Implementation
/////////////////////////////////////////////////////////
Dirt::Dirt(ActorWorld* world, double startX, double startY) : Damageable(world, IID_DIRT, startX, startY)
{}
    
    // Dirt does nothing every tick - just return
void Dirt::doSomething() {
    return;
}

    // Identifier
bool Dirt::isBlocker() const {
    return true;
}

/////////////////////////////////////////////////////////
// Projectile Implementation
/////////////////////////////////////////////////////////
Projectile::Projectile(ActorWorld* world, int imageID, double startX, double startY, Direction startDir, int maxTravelDist, int damageToken)
: Actor(world, imageID, startX, startY, startDir, 1), m_maxTravelDistance(maxTravelDist), m_distanceTravelled(0), m_damageToken(damageToken)
{}

    // All Projectiles start the tick by checking if they're alive
void Projectile::doSomething() {
    if (!isAlive())
        return;
    
    specificProjectileAction();
    
    moveAngle(getDirection(), SPRITE_WIDTH);
    travelDistance(SPRITE_WIDTH);
}

    // Identifier
bool Projectile::isProjectile() const {
    return true;
}

    // Accessors
int Projectile::getDamageToken() const {
    return m_damageToken;
}

    // Protected Accessors
int Projectile::distanceTravelled() const {
    return m_distanceTravelled;
}

    // Protected Mutators
void Projectile::travelDistance(int dist) {
    if (dist < 0)
        return;
    m_distanceTravelled += dist;
    
    if (m_distanceTravelled >= m_maxTravelDistance)
        setDead();
}

/////////////////////////////////////////////////////////
// Flame Implementation
/////////////////////////////////////////////////////////
Flame::Flame(ActorWorld* world, double startX, double startY, Direction startDir) : Projectile(world, IID_FLAME, startX, startY, startDir, 32, 5)
{}

void Flame::specificProjectileAction() {
    getWorld()->flameDamage(this);
}

/////////////////////////////////////////////////////////
// Spray Implementation
/////////////////////////////////////////////////////////
Spray::Spray(ActorWorld* world, double startX, double startY, Direction startDir) : Projectile(world, IID_SPRAY, startX, startY, startDir, 112, 2)
{}

void Spray::specificProjectileAction() {
    getWorld()->sprayDamage(this);
}

/////////////////////////////////////////////////////////
// Food Implementation
/////////////////////////////////////////////////////////
Food::Food(ActorWorld* world, double startX, double startY) : Actor(world, IID_FOOD, startX, startY, up, 1)
{}

    // Food does nothing every tick
void Food::doSomething() {
    return;
}

    // Identifier
bool Food::isEdible() const {
    return true;
}

/////////////////////////////////////////////////////////
// Pit Implementation
/////////////////////////////////////////////////////////
Pit::Pit(ActorWorld* world, double startX, double startY) : Actor(world, IID_PIT, startX, startY, right, 1), m_regSalLeft(5), m_aggSalLeft(3), m_eColiLeft(2)
{
//    m_bacteriaInv[0] = 5;  // element 0 represents RegularSalmonella yet to spawn
//    m_bacteriaInv[1] = 3;  // element 1 represents AggressiveSalmonella yet to spawn
//    m_bacteriaInv[2] = 2;  // element 2 represents EColi yet to spawn
}

    // Pits have a chance to spawn Bacteria every tick
void Pit::doSomething() {
    if (!isAlive())
        return;
    
    if (isPitEmpty()) {
        setDead();
        return;
    }
    
    int chance = randInt(1, 50);
    
    if (chance == 1 && !isPitEmpty()) {
        int indexOfBacteria = spawnWhichBacteria();
        
        switch (indexOfBacteria) {
            case 0:
                getWorld()->spawnRegSal(getX(), getY());
                m_regSalLeft--;
                break;
                
            case 1:
                getWorld()->spawnAggSal(getX(), getY());
                m_aggSalLeft--;
                break;
                
            case 2:
                getWorld()->spawnEColi(getX(), getY());
                m_eColiLeft--;
                break;
        }
        
        getWorld()->playSound(SOUND_BACTERIUM_BORN);
    }
}

    // Identifier
bool Pit::isBacteriaSpawner() const {
    return true;
}

    // Mutator
void Pit::setDead() {
    Actor::setDead();
    getWorld()->decPitCount();
}

    // Private Auxiliary Functions
bool Pit::isPitEmpty() const {
    return m_regSalLeft == 0 && m_aggSalLeft == 0 && m_eColiLeft == 0;
}

int Pit::spawnWhichBacteria() const {
    if (m_regSalLeft == 0 && m_aggSalLeft == 0) {
        return 2;
    } else if (m_regSalLeft == 0 && m_eColiLeft == 0) {
        return 1;
    } else if (m_aggSalLeft == 0 && m_eColiLeft == 0) {
        return 0;
    } else if (m_regSalLeft == 0) {
        return randInt(1, 2);   // either AggressiveSalmonella or EColi
    } else if (m_aggSalLeft == 0) {
        int retVal = randInt(4, 5);    // either RegularSalmonella or EColi
        if (retVal == 4)               // randInt gives us a random integer between the params, so we have to use placeholder values
            return 0;
        if (retVal == 5)
            return 2;
    } else if (m_eColiLeft == 0) {
        return randInt(0, 1);
    } else {
        return randInt(0, 2);
    }
    
    return -1;  // function should never get here
}
