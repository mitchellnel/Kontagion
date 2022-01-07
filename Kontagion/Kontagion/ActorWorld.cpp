#include "ActorWorld.h"
#include "GameConstants.h"
#include "Actor.h"

#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
using namespace std;

GameWorld* createActorWorld(string assetPath)
{
	return new ActorWorld(assetPath);
}

ActorWorld::ActorWorld(string assetPath) : GameWorld(assetPath), socrates(nullptr), m_bacteria(0), m_pits(0)
{}

ActorWorld::~ActorWorld() {
    cleanUp();
}

/////////////////////////////////////////////////////////////////
// General Auxiliary Functions
/////////////////////////////////////////////////////////////////
void ActorWorld::decPitCount() {
    m_pits--;
}

void ActorWorld::decBacteriaCount() {
    m_bacteria--;
}

bool ActorWorld::finishedLevel() const {
    return m_pits == 0 && m_bacteria == 0;
}

///////////////////////////////////////////////////////////////////
// Socrates Auxiliary Functions
///////////////////////////////////////////////////////////////////
bool ActorWorld::socratesOverlap(Actor* a) {
    return a->overlaps(a, socrates);
}

//////////////////////////////////////////////////////////////////////
// Bacteria Auxiliary Functions
//////////////////////////////////////////////////////////////////////
    // Checks whether the passed in Bacteria overlaps with Socrates, damaging Socrates if it does
bool ActorWorld::bacteriaSocratesOverlap(Bacteria* bacteria) {
    if (socratesOverlap(bacteria)) {
        socrates->damageCharacter(bacteria->getDamageToken());
        return true;
    }
    return false;
}

bool ActorWorld::bacteriaMovementBlocked(const double& attemptX, const double& attemptY) const {
    for (auto itr = actors.begin(); itr != actors.end(); ) {
        if ((*itr)->isAlive() && (*itr)->isBlocker() && (*itr)->movementOverlap(attemptX, attemptY, *itr)) {
            return true;
        }
        else
            itr++;
    }
    return false;
}

    // Returns true and updates socratesX and socratesY with Socrates's coordinates if the passed in Bacteria is <= a specified distance from Socrates
bool ActorWorld::nearSocrates(Bacteria* bacteria, double& socratesX, double& socratesY, const double& dist) {
    if (bacteria->distance(bacteria, socrates) <= dist) {
        socratesX = socrates->getX();
        socratesY = socrates->getY();
        return true;
    }
    return false;
}

bool ActorWorld::canEatFood(Bacteria* bacteria) {
    for (auto itr = actors.begin(); itr != actors.end(); ) {
        if ((*itr)->isAlive() && (*itr)->isEdible() && bacteria->overlaps(bacteria, *itr)) {
            (*itr)->setDead();
            return true;        // the Bacteria will then eat the Food in this function's caller
        } else {
            itr++;
        }
    }
    return false;
}



bool ActorWorld::findFood(Bacteria* bacteria, double& foodX, double& foodY) {
    int minDistToFood = 129;    // Food must be within 128 pixels of the Bacteria for it to pathfind to the food
    Actor* nearestFood = nullptr;
    for (auto itr = actors.begin(); itr != actors.end(); itr++) {
        if ((*itr)->isAlive() && (*itr)->isEdible() && bacteria->distance(bacteria, *itr) <= 128) {
            if (minDistToFood > bacteria->distance(bacteria, *itr)) {
                minDistToFood = bacteria->distance(bacteria, *itr);
                nearestFood = *itr;
            }
        }
    }
    
    if (minDistToFood != 129 && nearestFood != nullptr) {
        foodX = nearestFood->getX();
        foodY = nearestFood->getY();
        return true;
    } else {
        return false;
    }
}

    // Every Bacteria has a 50% chance to drop a Food item at their position of death
void ActorWorld::dropFood(double x, double y) {
    int chance = randInt(0, 1);
    if (chance == 1)
        actors.push_back(new Food(this, x, y));
}

/////////////////////////////////////////////////////////////////
// Bacteria Spawning Functions
/////////////////////////////////////////////////////////////////
void ActorWorld::spawnRegSal(double startX, double startY) {
    actors.push_back(new RegularSalmonella(this, startX, startY));
    m_bacteria++;
}

void ActorWorld::spawnAggSal(double startX, double startY) {
    actors.push_back(new AggressiveSalmonella(this, startX, startY));
    m_bacteria++;
}

void ActorWorld::spawnEColi(double startX, double startY) {
    actors.push_back(new EColi(this, startX, startY));
    m_bacteria++;
}

///////////////////////////////////////////////////////////////
// Flame Auxiliary Functions
///////////////////////////////////////////////////////////////
void ActorWorld::throwFlames() {
    for (int i = 0; i < 16; i++) {
        double flameX, flameY;
        socrates->getPositionInThisDirection(i*22, SPRITE_WIDTH, flameX, flameY);
        actors.push_front(new Flame(this, flameX, flameY, i*22));
    }
}

void ActorWorld::flameDamage(Projectile* flame) {
    for (auto itr = actors.begin(); itr != actors.end(); ) {
        if ((*itr)->isAlive() && (*itr)->isDamageable() && flame->overlaps(flame, *itr)) {
            if ((*itr)->isCharacter()) {
                Character* theCharacter = static_cast<Character*>(*itr); // *itr is guaranteed to be a Character
                theCharacter->damageCharacter(flame->getDamageToken());
            } else {
                (*itr)->setDead();
            }
            
            flame->setDead();
            return;
        } else {
            itr++;
        }
    }
}

///////////////////////////////////////////////////////////////
// Spray Auxiliary Functions
///////////////////////////////////////////////////////////////
void ActorWorld::sprayDisinfectant() {
    double sprayX, sprayY;
    socrates->getPositionInThisDirection(socrates->getDirection(), SPRITE_WIDTH, sprayX, sprayY);
    actors.push_front(new Spray(this, sprayX, sprayY, socrates->getDirection()));
}

void ActorWorld::sprayDamage(Projectile* spray) {
    for (auto itr = actors.begin(); itr != actors.end(); ) {
        if ((*itr)->isAlive() && (*itr)->isDamageable() && spray->overlaps(spray, *itr)) {
            if ((*itr)->isCharacter()) {
                Character* theCharacter = static_cast<Character*>(*itr); // *itr is guaranteed to be a Character
                theCharacter->damageCharacter(spray->getDamageToken());
            }
            else {
                (*itr)->setDead();
            }
            
            spray->setDead();
            return;
        } else {
            itr++;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////
// HealthGoodie Auxiliary Function
/////////////////////////////////////////////////////////////////////////////////
void ActorWorld::healSocrates() {
    socrates->restoreHealthToFull();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// FlameGoodie Auxiliary Function
//////////////////////////////////////////////////////////////////////////////////////////////
void ActorWorld::rechargeFlamethrower() {
    socrates->chargeFlamethrower();
}

/////////////////////////////////////////////////////////////////
// Fungus Auxiliary Function
/////////////////////////////////////////////////////////////////
void ActorWorld::fungusHurtSocrates() {
    socrates->damageCharacter(20);
}

/////////////////////////////////////////////////
// Supporting Functions
/////////////////////////////////////////////////
void ActorWorld::generateRandPos(double& x, double& y) {
    // Generate a random angle and radius to generate a random position that pits, food and dirt can spawn at in a circle of radius 120, with centre (128, 128)
    double angle = (static_cast<double>(randInt(0, 100)) / 100) * 2 * M_PI;
    double radius = sqrt(static_cast<double>(randInt(0, 100)) / 100) * 120;
    x = radius * cos(angle) + 128;
    y = radius * sin(angle) + 128;
}

void ActorWorld::generateRandPosOnBorder(double& x, double& y) {
    x = randInt(0, VIEW_WIDTH);
    if (randInt(0, 1) == 1)     // 50% chance for each possible y value
        y = -sqrt((128 * 128) - pow(x - 128, 2)) + 128;
    else
        y = sqrt((128 * 128) - pow(x - 128, 2)) + 128;
}

void ActorWorld::removeDeadActors() {
    for (auto itr = actors.begin(); itr != actors.end(); ) {
        if (!(*itr)->isAlive()) {
            delete *itr;
            itr = actors.erase(itr);
        } else {
            itr++;
        }
    }
}

void ActorWorld::addGoodiesOrFungi() {
    int changeFungus = max(510 - getLevel() * 10, 200);
    int rand = randInt(0, changeFungus-1);              // [0, chanceFungus)
    
    if (rand == 0) {
        double randX, randY;
        generateRandPosOnBorder(randX, randY);
        
        actors.push_back(new Fungus(this, randX, randY));
    }
    
    int chanceGoodie = max(510 - getLevel() * 10, 250);
    rand = randInt(0, chanceGoodie-1);                  // [0, changeGoodie)
    
    if (rand == 0) {
        double randX, randY;
        randX = randInt(0, VIEW_WIDTH);
        generateRandPosOnBorder(randX, randY);
        
        switch(randInt(1, 10)) {    // random number from a set of 10 elements
            case 1:     // 1 element == 10% chance
                actors.push_back(new LifeGoodie(this, randX, randY));
                break;
                
            case 2:
            case 3:
            case 4:     // 3 elements == 30% chance
                actors.push_back(new FlameGoodie(this, randX, randY));
                break;
                 
            default:    // remaining elements == 60% chance
                actors.push_back(new HealthGoodie(this, randX, randY));
                break;
        }
    }
}

void ActorWorld::updateStatusText() {
    ostringstream statusText;
    statusText.fill('0');
    statusText << "Score: " << setw(6) << getScore();
    statusText.fill(' ');
    statusText << "  Level: " << setw(2) << getLevel();
    statusText << "  Lives: " << setw(1) << getLives();
    statusText << "  Health: " << setw(3) << socrates->getHealth();
    statusText << "  Sprays: " << setw(2) << socrates->spraysRemaining();
    statusText << "  Flames: " << setw(2) << socrates->flamesRemaining();
    string status = statusText.str();
    setGameStatText(status);
}

/*////////////////////////////////////////////////////////////////*/

/////////////////////////////////////
// Core Functions
/////////////////////////////////////

int ActorWorld::init()
{
    m_bacteria = 0;
    m_pits = 0;
    
    socrates = new Socrates(this);
    
    for (int i = 0; i < getLevel(); i++) {
        double x, y;
        generateRandPos(x, y);
        
        if (getLevel() == 1) {
            actors.push_back(new Pit(this, x, y));
            m_pits++;
            break;
        }
        
        Actor* newPit = new Pit(this, x, y);
        
            // Check for overlaps between the newPit and any existing Pits
        for (auto itr = actors.begin(); itr != actors.end(); ) {
            if (newPit->overlaps(newPit, *itr)) {
                delete newPit;
                generateRandPos(x, y);
                newPit = new Pit(this, x, y);
            } else {
                itr++;
            }
        }
        
        actors.push_back(newPit);
        m_pits++;
    }
    
    for (int i = 0; i < min(5*getLevel(), 25); i++) {
        double x, y;
        generateRandPos(x, y);
        
        Actor* newFood = new Food(this, x, y);
        
            // Check for overlaps between the newFood and any existing Pits or Food
        for (auto itr = actors.begin(); itr != actors.end(); ) {
            if (newFood->overlaps(newFood, *itr)) {
                delete newFood;
                generateRandPos(x, y);
                newFood = new Food(this, x, y);
            } else {
                itr++;
            }
        }
        
        actors.push_back(newFood);
    }
    
    for (int i = 0; i < max(180-20*getLevel(), 20); i++) {
        double x, y;
        generateRandPos(x, y);
        
        Actor* newDirt = new Dirt(this, x, y);
        
            // Check for overlaps between the newDirt and any existing Pits or Food
        for (auto itr = actors.begin(); itr != actors.end(); ) {
            if (!(*itr)->isBlocker() && newDirt->overlaps(newDirt, *itr)) {    // Dirt can overlap with each other
                delete newDirt;
                generateRandPos(x, y);
                newDirt = new Dirt(this, x, y);
            } else {
                itr++;
            }
        }
        
        actors.push_back(newDirt);
    }
    
    return GWSTATUS_CONTINUE_GAME;
}


int ActorWorld::move()
{
    if (socrates->isAlive())
        socrates->doSomething();
    else
        return GWSTATUS_PLAYER_DIED;    // this should never actually be called here; just an invariant check
    
        // Traverse through list, letting all Actors do something if they're alive
    for (auto itr = actors.begin(); itr != actors.end(); itr++) {
        if ((*itr)->isAlive()) {
            (*itr)->doSomething();
            
            if (!socrates->isAlive()) {
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }

            if (finishedLevel())
                return GWSTATUS_FINISHED_LEVEL;
        }
    }
    
    if (m_pits == 0 && m_bacteria == 0)
        return GWSTATUS_FINISHED_LEVEL;
    
    removeDeadActors();
    
    addGoodiesOrFungi();
    
    updateStatusText();
    
    return GWSTATUS_CONTINUE_GAME;
    
}

void ActorWorld::cleanUp()
{
    delete socrates;
    socrates = nullptr;

    for (auto itr = actors.begin(); itr != actors.end(); ) {
        delete *itr;
        itr = actors.erase(itr);
    }
}
