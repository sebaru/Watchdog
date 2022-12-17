# Abls-Habitat Project

Abls-Habitat is my own project to do home automation. it presents:

* one or more [Agents](https://github.com/sebaru/abls-habitat-agent), in house, to interact with sensors and outputs
* one [API](https://github.com/sebaru/abls-habitat-api) on SaaS, main process of project, to handle all of agents
* one [Console](https://github.com/sebaru/abls-habitat-console) to configure each element and develop [D.L.S module](https://docs.abls-habitat.fr/)
* one [Home](https://github/com/sebaru/abls-habitat-home) frontend for all users

This software is Work In Progress. It is a complete refund of all-in-one Watchdog Project.
I'm developing on my sparse-time, not so easy with little kid :-).

All detailed documentations [are here](https://docs.abls-habitat.fr)

QuickStart:

    git clone https://github.com/sebaru/Watchdog.git
    cd Watchdog
    sudo ./INSTALL.sh

Have a good day, Sebaru.

## What is Watchdog

This component is the legacy all in one program for my home automation.
I'm going to move parts of it into API, Console and HOME. Goal is to only keep code for dealing with sensors and outputs.

This repository will be renamed to abls-habitat-agent when I will be ready.
