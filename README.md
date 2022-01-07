# Kontagion
Kontagion is a single-player, sprite-based graphics game (externally provided).

#### Concepts:
* Object-oriented programming
* Inheritance
* Data structures

Here is a screenshot of the Kontagion game, with Socrates, Dirt, EColi, Salmonella, the bacterial pit, and some food objects present:

<img width="780" alt="Kontagion" src="https://user-images.githubusercontent.com/56947176/94908889-eb2b6680-04d4-11eb-8a9b-26fe6a179afd.png">

Goodies can spawn around the edge of the arena, and have power-ups for Socrates, such as restoring health, giving him flame charges, or giving him another life.

Fungi spawns similarly, but does damage to Socrates instead.

## Program Design and Class Descriptions
I chose to use an STL List to implement my actors container in the ActorWorld class. I did this because my programme did not require random access into the container, and due to the advantages STL Lists present when it comes to erasing from the middle of the container (versus a Vector).
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

Actor contains 8 virtual identifier functions, that all return false by default, and are implemented to return true in the class they are identifying, i.e. isGoodie() returns false for a Character object, but true for a Goodie object. These functions were chosen to be virtual as depending on the derived type, it will have a different implementation. Actor also contains an accessor that returns a pointer to the ActorWorld object each Actor is contained within, as well as an isAlive() function. It also contains a mutator, setDead(), which does as the name suggests, but is declared virtual in order to allow derived types to play their own death-associated sound, and in Bacteria’s case, update certain details in the ActorWorld object.

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

Character has an identifier function, isCharacter(). This function is crucial for usage in ActorWorld, not only to decide whether or not to call damageCharacter() (seen below), but also to ensure guarantees that static casts from an Actor pointer to a Character pointer would not result in undefined behaviour, which would facilitate the behaviour and definition of the damageCharacter() function.

Character also has an accessor, getHealth(), which returns the Character’s current health value. It also contains a protected mutator, setHealth(), which sets the Character’s health to a specific value. The choice to make this protected was to ensure that it could not be called anywhere but internally in Character objects.

Character’s key function is a virtual damageCharacter() function that takes an int value representative of the damage to inflict, and returns nothing. This function inflicts the specified damage to the character, and sets them to dead if they run out of health. This function was chosen to be virtual to facilitate the implementation of sound playing in response to inflicted damage or death for Socrates and the Bacteria types.

### Socrates
Socrates is the player. Socrates has no identifier, as ActorWorld stores a pointer directly to the Socrates object, and thus has no need to be picked out from other Actors in the actors list.
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

He also has two auxiliary functions, sprayDisinfectant() and throwFlames(), both of which call respective auxiliary functions in ActorWorld (that merely add spray or flame Actors to the actor list in accordance with the project specification), decrement respective charges and play respective sounds.

### Bacteria
Bacteria are the enemies in Kontagion. They have an identifier, isBacteria().

They also have an implementation of the doSomething() function, which checks if the calling Bacteria is alive, immediately returning if not. It then calls specificBacteriaAction(). This is a pure virtual function that Bacteria defines, which ensures each type of Bacteria implement their own specific actions in accordance with their type.

Bacteria also has accessors to get its damage token (the amount of damage each Bacteria inflicts upon Socrates), and two protected accessors that return values for the food the Bacteria has eaten, as well as its current movement plan distance.

Bacteria has its own implementation of setDead(), which updates the score and bacteria count in the ActorWorld object it belongs to, as well as potentially spawning a Food object when it dies.

Bacteria also has several protected mutators, incFoodEaten(), resetFoodEaten(), decMovemenPlanDist(), and resetMovementPlanDist(), which all do as their name implies.

Bacteria also has several protected auxiliaries:
*	divide(); a pure virtual function that implements the specifications requirements for Bacterial division, and is pure virtual to ensure each type of Bacteria divides into their own type (by having their own implementation)
*	attemptMove(); a function that sees whether movement by a specified distance is possible (will not be blocked by Dirt and does not move outside of the Petri dish) 
*	findDirectionTo(); a function that finds the direction to a target coordinate, where an Actor currently is
*	divideOrEat(); a function that decides whether a Bacterial object should divide or eat
*	attemptToDamageSocrates(); a function that attempts to damage Socrates if the Bacteria overlaps with it, and then calls divideOrEat() if that is not possible

### Salmonella
There are two types of Salmonella that share common functionalities.

The two types of Salmonella have very similar methodologies to their actions every tick, so specificBacteriaAction() is implemented in this class as such:
```
aggress if salmonella is aggressive, if aggression successful, return immediately
try and damage Socrates or divide or eat
move if possible, if movement if successful return immediately
find path to nearest food object
```

It also has its own implementation of damageCharacter() that calls the base class version, and plays Salmonella specific sounds for being hurt or being killed.

It contains several protected auxiliaries:
*	aggressivePathfind(); a pure virtual function that implements the aggressive behaviour of the two salmonella types, with them having unique implementations (i.e. RegularSalmonella just returns false immediately as it is not aggressive)
*	salmonellaMovementAttempt(); a function that attempts the move a salmonella can make every tick
*	foodPathfind(); a function that allows Salmonella to find a path (really a direction) to the nearest Food object
*	selectNewDir(); a function that selects a random direction for the Salmonella object

### RegularSalmonella
RegularSalmonella implements its own protected auxiliary to divide: it merely calls the base class divide function but specifically makes a call to ActorWorld to spawn a new RegularSalmonella object. It also implements aggressivePathfind with an immediate return of false, as it is not aggressive.

### AggressiveSalmonella
AggressiveSalmonella also implements its own protected auxiliary to divide: it merely calls the base class divide function but specifically makes a call to ActorWorld to spawn a new AggressiveSalmonella object. It also implements aggressivePathfind with an algorithm to find the direction to Socrates if possible, then try to move to Socrates, and then try to damage him. Returning a Boolean value depending on whether the aggression is successful to implement specification requirements.

### EColi
EColi implements its own variation of the specificBacteriaAction() function with the short pseudocode:
```
try to damage Socrates
set direction towards Socrates and try to move towards him
```

It also has its own implementations of damageCharacter() that calls the base class version, and plays EColi specific sounds for being hurt or being killed. As well as a divide function that calls the base class divide function but specifically makes a call to ActorWorld to spawn a new EColi object.

### Goodie
Goodies are damageable, but are unique in the way they affect Socrates. It has an identifier, isGoodie.

Goodie implements its own variation of doSomething() as required, checking if the Goodie is alive and returning otherwise, and then taking specific Goodie actions (dependent on the derived type) on Socrates if he is overlapping with the Goodie, and setting itself to dead, or decreasing its remaining time in existence.

It has a pure virtual specificGoodieAction() that plays the got goodie sound and increases its score by the score value of the derived type. It is pure virtual as each derived class of Goodie has its own action it performs on Socrates.

It has a mutator that decrements its remaining time in existence, and an accessor that gets the specific Goodie’s score value (with each derived type of Goodie specifying what this is upon construction).

### HealthGoodie
HealthGoodie’s specificGoodieAction() implementation calls the base class version, and makes a call to the ActorWorld object to restore Socrates’s health to full.

### FlameGoodie
FlameGoodie’s specificGoodieAction() implementation calls the base class version, and makes a call to the ActorWorld object to increase Socrate’s flamethrower charges by 5.

### LifeGoodie
LifeGoodie’s specificGoodieAction() implementation calls the base class version, and makes a call to the ActorWorld object to increment Socrates’s current lives.

### Fungus
Fungus’s specificGoodieActions() does not call the base class version. It instead completely overrides the functionality to prevent a sound from picking up the Fungus from playing. It makes a call to the ActorWorld object to decrease the player score and hurt Socrates.

### Dirt
Dirt can also be damaged, but blocks movement by Bacteria, so has an isBlocker() identifier.
It’s doSomething() method immediately returns.

### Projectile
Projectiles inherit from Actor as they cannot be damaged, but instead inflict damage. It has an identifier, isProjectile().

All projectiles do similar things, so its doSomething() implementation reflects this. It starts by checking if the Projectile is alive, returning immediately if not, and then undertaking actions specific to the type of Projectile it is, and then making updates to its position and travelled distance.

It also has an accessor that returns its damage token, which is specified at construction, that is used to work out how much damage to inflict to a Character.

It has a protected accessor to its distance travelled, and a travelDistance() mutator. The mutator updates the travelled distance member variable, and also checks if the Projectile has met or exceeded its maximum travel distance (which is specified at construction of the derived types).

### Flame
Flame’s specificProjectileAction() is to make a call to ActorWorld to cause flame damage to any Actor that overlaps with the Flame.

### Spray
Spray’s specificProjectileAction() is to make a call to ActorWorld to cause spray damage to any Actor that overlaps with the Spray.

### Food
Food cannot be damaged but can be eaten by Bacteria for division, so it has an isEdible() identifier.
It’s doSomething() implementation immediately returns.

### Pit
Pits are the bacteria spawner in Kontagion. It has an identifier, isBacteriaSpawner().
It’s doSomething() implementation checks if the Pit is alive, returning immediately if not, and also checks if the Pit is empty, setting it to dead and immediately returning if it is. It also has a 1 in 50 chance each tick to spawn a Bacteria, of which each type is equally likely to be spawned on any given tick.

## Testing Procedures
I tested the functionality of my version of Kontagion through a variety of assert statements and playing of the game. 

As a general test, I played my version of the game thrice, and then compared it to the source version of the game, which I also played thrice. On each playthrough, I noted different parts of the programme. On the first run, I noted the initialisation of the ActorWorld, and saw that it had identical behaviour. On the second, I noted the behaviours of the Bacterium, and believe that they matched, and also interacted with them using Socrates’s projectiles, thereby testing those as well. On the third playthrough, I experimented with the Goodies, destroying them or picking them up whenever I had the chance.

Note that if a class had a pure virtual function and required instantiation to test, I temporarily removed the pure virtual function to test it.

### Actor
I tested the Actor class by setting up ActorWorld in a way that allowed me to assert the correct return values of my overlapping and distance auxiliary functions, my Actor identifiers for the foundational base class, the getWorld() and isAlive() accessors, and the setDead() mutator.

### Damageable
I tested the Damageable identifier using an assert statement. I then tested all of the Damageable objects by playing the game and trying to destroy them with Socrates’s spray and flame projectiles. I also continued to test aspects of the base Actor class with assert statements.

### Character
The Character class itself was not difficult to test, I merely used assert statements to test its accessor, mutators and identifier. I also continued to test aspects of its base classes with assert statements, such as ensuring the overlapping functions were still working.

### Socrates
Socrates is the player, so my main way to test him was by playing the game. That being said, I used assert statements to test the properties I knew he would have (i.e. correct returns for any base class methods), as well as ensure he was being properly damaged and was properly overlapping with other Characters (which I made dummy versions of).

By playing the game, I tested his interactions with Goodies and other Bacteria that would try and damage him, and saw that everything was working as intended. I did similar things with Dirt piles, using this to also test his sprays and flames, and utilising the freezing and tick by tick observation of the game to ensure they were travelling the correct distance.

### Bacteria
My main way to test the Bacteria class itself was to assert the mathematics behind all of its auxiliaries, as well as assert its accessors, mutators and identifiers. Crucially, I made sure its setDead function was updating ActorWorld properly. There was not much else to test for this base class.

### Salmonella
To test the Salmonella class I made sure that damageCharacter was working as intended, as well as playing the correct sounds upon Salmonella damage or death. Its specificBacteriaAction() function was tested by observing the way a RegularSalmonella would move through the course of the game. I set up a makeshift ActorWorld with some Dirt and Food objects to see if the Salmonella would behave as expected. I also used the freezing and tick-by-tick observation of my game compared to the source game to see if the behaviours were identical. I made sure its setDead function played the right sound, and its damageCharacter function played the right sound at the right moment.

### RegularSalmonella
For RegularSalmonella, its movement is the same as the Salmonella base class, so I only had to test the divide function by forcing the RegularSalmonella to divide, and checking the position of where the new RegularSalmonella spawned. I made sure its setDead function played the right sound, and its damageCharacter function played the right sound at the right moment.

### AggressiveSalmonella
For AggressiveSalmonella, the crux of its movement is the same as the Salmonella base class. I tested the aggressivePathfind function by creating a makeshift ActorWorld that had food in it as well as Socrates. I also used the freezing and tick-by-tick observation of my game compared to the source game to see if the behaviours were identical. I also tested the divide function in a similar way to RegularSalmonella. I made sure its setDead function played the right sound, and its damageCharacter function played the right sound at the right moment.

### EColi
As EColi had a unique movement action, I tested it by creating a makeshift ActorWorld that had food in it as well as Socrates. I also used the freezing and tick-by-tick observation of my game compared to the source game to see if the behaviours were identical. I also tested the divide function in a similar way to RegularSalmonella.

### Goodie
For Goodies, I tested them by trying to destroy them as I played the game, as well as picking them up and seeing if a sound was played. I also roughly compared the spawn rates of Goodies with that of the source game, and didn’t notice any problems. I also set specific lifetimes for each Goodie to ensure that was working as intended.

### HealthGoodie
HealthGoodie was tested the same way as its base class, but I also made sure to assert Socrates’s properties had been updated properly.

### FlameGoodie
FlameGoodie was tested the same way as its base class, but I also made sure to assert Socrates’s properties had been updated properly.

### LifeGoodie
LifeGoodie was tested the same way as its base class, but I also made sure to assert Socrates’s properties had been updated properly.

### Fungus
Fungus was tested the same way as its base class, but I also made sure to assert Socrates’s properties had been updated properly. In addition, I made sure that no sound was played upon picking it up, which is contrary to what the base class defines.

### Dirt
Dirt did nothing every tick. This was easy to test. But I made sure that it was destroyable using Socrates’s spray and flames.

### Projectile
Projectiles were tested the tick-by-tick observation of the game to ensure that they travelled their specified distance (how many ticks they lasted for).

### Flame
Flame was tested as specified in the base class, but I also used the tick-by-tick observation to ensure the correct number of Flames were generated upon a single enter press. I also used assert statements to ensure they were doing the right amount of damage.

### Spray
Spray was tested as specified in the base class, but I also used assert statements to ensure they were doing the right amount of damage.

### Food
Food did nothing every tick. This was easy to test. But I made sure that it was being picked up by bacteria that ran over it, and that their food eaten counts were increasing accordingly.

### Pit
Pits were tested using assert statements to ensure that the correct number of Bacteria were being produced over a Pit’s lifetime.
