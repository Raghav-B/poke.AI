# poke.AI

An experimental project to attempt to create an AI that can play the 3rd generation Pokemon games (specifically Pokemon Emerald - it's very close to my heart).

<p align = "center">
    <img src="readme_src/mapping_trial.png" alt="Prototype of mapping with detection">
</p>

## Introduction

A little bit of a disclaimer before we continue - it is very unlikely that given current technology we can create an AI that can actually *learn* how to play any of the Pokemon games from first principles. This is an issue with most open-world games as they have many mini-games (or rather mechanics) that come together to create the complete game. 

Although we have seen development in the field of Reinforcement Learning with regards to teaching a [neural network how to play the Atari games](https://arxiv.org/abs/1312.5602) using Q-Learning or Deep Q-Learning, these games are relatively simplistic in scale when compared to the complexity of Pokemon Emerald, so much so that a single convolutional neural network can be used to model these Atari games.

To further illustrate this point, here are the core mechanics of Space Invaders as compared to Pokemon Emerald:

| Space Invaders | Pokemon Emerald |
| --- | --- |
| Shoot aliens, dodge bullets, get points | Explore the world |
| . | Collect Pokemon and items |
| . | Engage in Pokemon battles |
| . | *and a whole host of other mechanics* |

It's possible I haven't done Space Invaders enough justice but this should still illustrate my point (I grew up with Pokemon, not Space Invaders so my bias is completely justified). Furthermore, each of these mechanics contain a whole host of sub-mechanics ranging from pseudo-metroidvania style gameplay when unlocking new paths in the open world, learning the Pokemon-type meta-game, coming up with killer movesets, etc.

And so the difficulty arises in trying to figure out how a single AI can be used to learn all these different mechanics and nuances. Well, the answer is fairly clear for now - we design a separate module to handle each aspect of the game differently. 

_BUT WAIT RAGHAV - THAT ISN'T AI, WHAT ABOUT MY DREAMS OF CORTANA/SKYNET/HAL/JUST MONIKA?!_

No, of course it isn't - not with that definition of AI. Until we achieve the holy grail of an [Artificial General Intelligence](https://en.wikipedia.org/wiki/Artificial_general_intelligence) we are very far from the AI often seen in popular culture. (Although some would say that Deep Reinforcement Learning is one step in the right direction)

By letting go of our obsession with AI being this omniscient, mystical being, we allow ourselves to see that in its current state, AI can still be used to automate many tasks, and can learn patterns or draw distinctions that way beyond what can be possibly conceived by the human brain - thanks in part to how artificial neural networks are [universal function approximators](https://en.wikipedia.org/wiki/Universal_approximation_theorem).

So what does this mean for this Pokemon AI? Well yes, there are many parts of this project that will be automated and will leverage on the power of neural networks. However AI and computers in general are dumb, and so there is definitely an element of providing supervision and foundational instructions to these algorithms. Nothing to be ashamed about, this is just the current state of AI.

Anyways, I've rambled on for too long, lets dive into the technical details. You can find most of the main code [right here.](object_detection/keras-retinanet/ai/)

## Proposed Features / Modules

### Localization and Mapping of Game World

This is at the absolute core of our AI. Without being able to localize itself in the game and without storing locations of objects and places of interest in memory, our agent won't be able to do anything apart from moving around randomly. (Interestingly, given how probability works, there probably exists a parallel universe somewhere where my AI has finished the game from start to finish purely on randomness alone) Hence we have to figure out a way to give our agent access to this information, because this is exactly that a human player does when playing any game.

Bringing the analogy of a human player further, we can use a Convolutional Neural Network (CNN) to perform object detection on every frame of gameplay to detect objects (also because believe it or not, finding the locations of these objects through the ROMs RAM is significantly harder. Also object detection is really cool to look at! - when it works).

For now, the CNN has been trained to detect the following objects:
* NPCs
* Assorted Houses
* Pokemon Gyms
* Pokecenters
* Pokemarts
* Exits (Unused for now)
The plan is to add more classes once the other core features have been developed.

<p align = "center">
    <img src="readme_src/detection1.png" alt="Running pure detection without automated movement">
</p>

So now our AI can see objects in its immediate environment, but how does it remember the locations of these objects? As soon as an object disappears from the frame and re-enters it at a later time, our agent has no way of knowing it is encountering a previously detected object again!

This is where I took a bit of inspiration from [SLAM (Simultaneous Localization and Mapping)](https://en.wikipedia.org/wiki/Simultaneous_localization_and_mapping). By using the movement of our agent (odometry), we try to estimate the expected location of objects that we have detected. However, we have an inherent advantage because there is nothing to estimate in mapping a Pokemon Game World - everything is fixed to a Tile on screen so all coordinates are absolute. So instead of estimation, all we have to do is store all our detected objects in a list somewhere, come up with a way to check if any newly detected objects already exist in our list, and converting the coordinates of the detectd objects from a local scale (our game screen) to the global scale (ground truth map of the entire game).

<p align = "center">
    <img src="readme_src/mapping2.png" alt="Drawing out a map of the game world on the global scale while moving around randomly">
</p>

Eventually I'll go into more detail about how the algorithm works. For now, you can dive right into my hopefully adequately documented code!

### Automated Movement in the Game World

**NOTE: Will update this soon, I've changed the mapping algorithm to something called frontier based exploration. Here's a preview of the results!**

<p align = "center">
    <img src="readme_src/frontier_mapping_prototype.png" alt="Drawing out a map of the game world on the global scale while moving around randomly">
</p>

Now that we have a mapping algorithm, our AI will know exactly where to go! - which might be what you're thinking, but that's wrong. Currently the agent follows a pre-defined set of sequential instructions, [or can move around randomly](https://www.youtube.com/watch?v=PQ_kMoVHZYc). How do we teach the AI how to move around?

After a bit of thinking, it dawned on me that there was no need to really use an AI for this part. We can simply create a policy for the agent to follow which aims to greedily move around the map, detecting new objects. I took a bit of inspiration from Reinforcement Learning for this portion - where our agent is instructed to move in a particular direction based on a greedy search policy, such that it always moves in directions with the highest density of high-value objects (gyms, NPCs) and away from regions that it has already explored.

Currently developing this part, will update once I've come up with a concrete solution.

### Learning to Battle Pokemon (And Win)

So far you might feel like you've been cheated. Our AI didn't really *learn* how to play Pokemon - I'm just giving it a general idea about what to do. If you feel this way too then you should find this section a lot more interesting. This is where I plan to incorporate Deep Q-Learning to the Pokemon battle system to get our agent to learn what to do in a fight to get the highest winrate possible.

This will be done based on a custom reward function and repeated Pokemon battles to train our neural network.

Again, seeing my time contraints and skill level, the battle system won't be fully explored, instead it will focus on training an agent to get as high a winrate as possible with a particular Pokemon with 4 fixed moves and the ability to heal. Think of this as a proof of concept - given enough battles with a particular Pokemon, our agent can learn the strongest and most reliable moves. For the sake of simplicity, we won't be looking into Pokemon type differences for now, though this is a clear area of exploration going into the future.

### Further Mechanics

For the first prototype/minimum viable product/whatever you like to call it, my focus is just on the 3 mechanics above, seeing as how core they are to the gameplay of Pokemon. Once they have been developed adequately, they can be taken further to incorporate more complex behavious such as entering houses, finding particular NPCs, taking cues from in-game text, etc.

## Libraries Used

* [VBA Rerecording](https://github.com/TASVideos/vba-rerecording) - windows-based GBA emulator
* [AdvanceMap](https://hackromtools.altervista.org/advance-map/) - modifying GBA ROM maps (apologies if I couldn't find original source)
* [PyAutoGUI](https://github.com/asweigart/pyautogui) - GUI automation for controlling emulator
* [MSS](https://github.com/BoboTiG/python-mss) - high FPS screencapture for object detection
* [fastgrab](https://github.com/mherkazandjian/fastgrab) - alternate high FPS screencapture
* [Keras RetinaNet](https://github.com/fizyr/keras-retinanet/blob/master/README.md) - model used for object detection
* [LabelImg](https://github.com/tzutalin/labelImg) - to make the task of labelImg images a litle less arduous <3
* [OpenCV](https://opencv.org/) - various image processing and computer vision uses
* [ZMQ](https://github.com/zeromq) - interprocess communication between C++ and Python

## Installation

Below are the installation instructions for setting up the test environment for this repo on a Windows 10 PC - assuming you have absolutely NONE of the dependencies installed. (I might have missed a few dependencies, but most major things should be here already - [Google](https://lmgtfy.com/?q=how+2+dependencie) exists for a reason!)

Also, I assume you're using a relatively recent NVIDIA GPU for this. (9-series and above).

This guide will help you install the following on your PC:

TODO: this.
