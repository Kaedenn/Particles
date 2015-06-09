# Particles
Inherited program for small to medium 2D particle simulations

## Overview
The Collision Detection Example Programs are a driver for a data
structure implementing efficient fully dynamic collision detection
between moving hard spheres.

## Requirements
The Collision Detection Example Programs require glut.

## Installation Guide
Type `make` to build the project. The default target includes debugging
symbols, named `./CollisionBoxTest.debug`.

Type `make FAST=1` to disable debugging and enable optimization, naming the target `./CollisionBoxTest`. This used to be the default target.

Type `make PROF=1` to compile with profile-generation (a la `gprof`). The target is named `./CollisionBoxTest`.

The default target and the FAST target can coexist without collisions. The FAST
and PROF targets use the same naming and will override the other.

## Running the driver program
The target (either CollisionBoxTest.debug or CollisionBoxTest) accepts numerous
command-line arguments:
| Argument | Effect |
| --------------------- | -------------------------------------- |
| `--size <int>`          | The edge length of the simulation cube |
| `--radius <int>`        | The radius of the interactive circle |
| `-r <FLOAT>`            | Color of the particles, red component |
| `-g <FLOAT>`            | Color of the particles, green component |
| `-b <FLOAT>`            | Color of the particles, blue component |
| `--gravity <FLOAT>`     | Strength of the downward gravity (default is off) |
| `--friction <FLOAT>`    | Strength of the frictional force (default is off) |
| `--speedrange <FLOAT>`  | Maximum speed for the randomly-generated particles |
| `--stopped`             | All particles start frozen. Equivalent to `--speedrange 0` |
| `--fps`                 | Display the FPS to the console |
| `--particle-gravity`    | Simulate gravity between the small particles |
