# Kontagion
Kontagion is a single-player, sprite-based graphics game (externally provided).

#### Concepts:
* Object-oriented programming
* Inheritance
* Data structures

## Program Design and Class Descriptions
I chose to use an STL List to implement my actors container in the StudentWorld class. I did this because my programme did not require random access into the container, and due to the advantages STL Lists present when it comes to erasing from the middle of the container (versus a Vector).
The 20 classes (not including GraphObject) in this project have the following hierarchy of inheritance:
```
GraphObject
  Actor
    Damageable
      Character
        Socrates
        Bacteria
          Salmonella
        RegularSalmonella
        AggressiveSalmonella
          EColi
      Dirt
      Goodie
        HealthGoodie
        FlameGoodie
        LifeGoodie
        Fungus
    Projectile
      Flame
      Spray
    Food
    Pit
```
### Actor
The Actor class inherits from GraphObject, which contains all the implementations necessary for displaying the Actor objects on the game screen.

The first function actor defines is the pure virtual doSomething() function. A choice was made to define this as pure virtual not only to ensure that all Actors implement their own doSomething() routines (as each Actor (excluding Dirt and Food) do something unique to their type.

Actor contains 8 virtual identifier functions, that all return false by default, and are implemented to return true in the class they are identifying, i.e. isGoodie() returns false for a Character object, but true for a Goodie object. These functions were chosen to be virtual as depending on the derived type, it will have a different implementation. Actor also contains an accessor that returns a pointer to the StudentWorld object each Actor is contained within, as well as an isAlive() function. It also contains a mutator, setDead(), which does as the name suggests, but is declared virtual in order to allow derived types to play their own death-associated sound, and in Bacteria’s case, update certain details in the StudentWorld object.

Actor also contains several auxiliary functions that had no need to be virtual, as their implementations would be identical for all derived classes:
*	overlaps(); which returns a Boolean value dependent on whether two Actors have overlapped with each other (their centres are less than SPRITE_WIDTH pixels apart
*	movementOverlap(); which returns a Boolean value dependent on whether a position an Actor has attempted to move to will cause a movement overlap (their centres are less than or equal SPRITE_WIDTH / 2
*	two types of distance() functions; one returns the Euclidean distance between two Actors, the other returns the Euclidean distance between a coordinate and an Actor
*	distanceFromCentre(); returns the Euclidean distance of a coordinate from the centre of the Petri dish

### Damageable
The Damageable class inherits from Actor, and is distinguished in the fact that objects of this type can take damage, and be killed in this way.

Damageable only has one member function, its identifier function, isDamageable(). This function is utilised to work out if an Actor is damageable and therefore decide whether to damage an object if it comes into contact with Socrates’s flames or sprays.

### Character
A Character is distinguished from other Damageable objects in the way that every Character will not instantly be killed if they come into contact with a damaging source, i.e. they have hitpoints, and can withstand multiple bouts of damage.

Character has an identifier function, isCharacter(). This function is crucial for usage in StudentWorld, not only to decide whether or not to call damageCharacter() (seen below), but also to ensure guarantees that static casts from an Actor pointer to a Character pointer would not result in undefined behaviour, which would facilitate the behaviour and definition of the damageCharacter() function.

Character also has an accessor, getHealth(), which returns the Character’s current health value. It also contains a protected mutator, setHealth(), which sets the Character’s health to a specific value. The choice to make this protected was to ensure that it could not be called anywhere but internally in Character objects.

Character’s key function is a virtual damageCharacter() function that takes an int value representative of the damage to inflict, and returns nothing. This function inflicts the specified damage to the character, and sets them to dead if they run out of health. This function was chosen to be virtual to facilitate the implementation of sound playing in response to inflicted damage or death for Socrates and the Bacteria types.

### Socrates
Socrates is the player. Socrates has no identifier, as StudentWorld stores a pointer directly to the Socrates object, and thus has no need to be picked out from other Actors in the actors list.
Socrates has a doSomething() method unique to his behaviours in a tick, with the following pseudocode:
```
if Socrates is alive immediately return
else get a key press from the user
  if the left arrow key is pressed, move clockwise
  if the right arrow key is pressed, move anti-clockwise
  if the space key is pressed, spray disinfectant
  if the enter key is pressed, emit a ring of flames
if no key is pressed, recharge sprays if Socrates hasn’t sprayed for a tick and he does not have the maximum spray charges
```
Socrates also has accessors to determine his remaining number of sprays and flames, and are named as such, as well as mutators that heal him to full (in response to picking up a RestoreHealthGoodie, or charging his flamethrower in response to picking up a FlameGoodie).

Socrates implements its own version of the damageCharacter() function, which calls the base class implementation of the function, and then plays a sound dependent on whether he was hurt or died in response to the damage.

He also has two auxiliary functions, sprayDisinfectant() and throwFlames(), both of which call respective auxiliary functions in StudentWorld (that merely add spray or flame Actors to the actor list in accordance with the project specification), decrement respective charges and play respective sounds.

### Bacteria
Bacteria are the enemies in Kontagion. They have an identifier, isBacteria().

They also have an implementation of the doSomething() function, which checks if the calling Bacteria is alive, immediately returning if not. It then calls specificBacteriaAction(). This is a pure virtual function that Bacteria defines, which ensures each type of Bacteria implement their own specific actions in accordance with their type.

Bacteria also has accessors to get its damage token (the amount of damage each Bacteria inflicts upon Socrates), and two protected accessors that return values for the food the Bacteria has eaten, as well as its current movement plan distance.

Bacteria has its own implementation of setDead(), which updates the score and bacteria count in the StudentWorld object it belongs to, as well as potentially spawning a Food object when it dies.

Bacteria also has several protected mutators, incFoodEaten(), resetFoodEaten(), decMovemenPlanDist(), and resetMovementPlanDist(), which all do as their name implies.

Bacteria also has several protected auxiliaries:
*	divide(); a pure virtual function that implements the specifications requirements for Bacterial division, and is pure virtual to ensure each type of Bacteria divides into their own type (by having their own implementation)
*	attemptMove(); a function that sees whether movement by a specified distance is possible (will not be blocked by Dirt and does not move outside of the Petri dish) 
*	findDirectionTo(); a function that finds the direction to a target coordinate, where an Actor currently is
*	divideOrEat(); a function that decides whether a Bacterial object should divide or eat
*	attemptToDamageSocrates(); a function that attempts to damage Socrates if the Bacteria overlaps with it, and then calls divideOrEat() if that is not possible

### Salmonella
