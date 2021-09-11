# Pomme-X
Educational Project - Cnam Engineering School - OS

## Overview
Pomme-X is an educational project, meant to teach us C and Linux.
The requirements specificatiosn are:
- a basic interpreter ;
- an intern structure of i-nodes and file tree ;
- interactions with the file system manager ;
- shell commands using these functions.

## The Team
Our team is composed of Hélène Charpentier, Alexandre Pelletier, and Victor Pons. We are in a work/study program of the Cnam's engineering school.

## Methodology
We started with a [drawio brainstorming](Brainstorm.drawio.xml), to decompose our tasks.
We then used the course material to achieve these tasks.
We used the MVP method, with planned releases if we can afford them.
We first needed to have a functional project, then to better it.

## Build
Instead of following the suggested tutorial (http://gl.developpez.com/tutoriel/outil/makefile/), we were advised to use cmake to generate a makefile as follow:

In the project directory, type

`mkdir build`

`cd build`

The makefile is generated with

`cmake ..`

To compile, type

`make`
