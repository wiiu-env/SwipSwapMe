# SwipSwapMe Plugin for the Wii U Plugin System [![Build Status](https://api.travis-ci.org/Maschell/SwipSwapMe.svg?branch=master)](https://travis-ci.org/Maschell/SwipSwapMe)

This little app can be used to swap the TV and DRC screen.

# Wii U Plugin System
This is a plugin for the [Wii U Plugin System (WUPS)](https://github.com/Maschell/WiiUPluginSystem/). To be able to use this plugin you have to place the resulting `.mod` file into the following folder:

```
sd:/wiiu/plugins
```
When the file is placed on the SDCard you can load it with [plugin loader](https://github.com/Maschell/WiiUPluginSystem/).

# Usage

When starting the app, you have the option to set your own button combo. Just press the button combo you want for 2 seconds.
Otherwise it will use the default settings (TV button).

Press the TV button (or your button combo) on the Gamepad to swap the screens. Thats all.

## Building

For building you need: 
- [wups](https://github.com/Maschell/WiiUPluginSystem)
- [wut](https://github.com/decaf-emu/wut)
- [libutilswut](https://github.com/Maschell/libutils/tree/wut) (WUT version) for common functions.

Install them (in this order) according to their README's. Don't forget the dependencies of the libs itself.

### Building using the Dockerfile
It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once
docker build . -t swipswapme-builder

# make 
docker run -it --rm -v ${PWD}:/project swipswapme-builder make

# make clean
docker run -it --rm -v ${PWD}:/project swipswapme-builder make clean
```