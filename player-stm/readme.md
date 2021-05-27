# STM32F407 FLAC Player
## Overview
Created as a project for Embedded Systems course (AGH UST).

After analysing multiple repositories created for this particular platform, it became apparent that most of the existing music players support only the MP3 format. In spite of its popularity, MP3 compromises sound quality due to its lossy compression.

FLAC, as a lossless format, does not suffer from this particular drawback and allows listener to enjoy music of high quality.

This project is an attempt at creating a STM32F407 FLAC music player.

## Used hardware and software
* [STM32F407G-DISC1](https://download.kamami.pl/p561343-stm32f4disco-prod-spec.pdf) - target platform 
* [STM32Cube IDE 1.6.0](https://www.st.com/en/development-tools/stm32cubeide.html) - environment for development, debugging and source code generation
* [Moserial](https://wiki.gnome.org/action/show/Apps/Moserial?action=show&redirect=moserial#The_moserial_Project) - terminal for platform connectivity
* [libFlac](https://xiph.org/flac/) - library used to decode FLAC files
* [Audacity](https://audacity.pl/) - used to create FLAC files and analyse decoded samples

## Project development

### Creation of base project
