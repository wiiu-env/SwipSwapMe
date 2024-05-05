# SwipSwapMe Plugin

This little plugin can be used to swap the TV and GamePad screen and offers different audio modes.

**When this plugin is enabled, the audio output of your Wii U is forced to stereo**

## Installation

(`[ENVIRONMENT]` is a placeholder for the actual environment name.)

1. Copy the file `swipswapme.wps` into `sd:/wiiu/environments/[ENVIRONMENT]/plugins`.
2. Requires the [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.
3. Requires the [MemoryMappingModule](https://github.com/wiiu-env/MemoryMappingModule) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.
4. Requires the [NotificationModule](https://github.com/wiiu-env/NotificationModule) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.

## Usage

Press a button combo (default is the GamePad's TV button) on the GamePad, Pro Controller or Classic Controller to take swap the screens.

Via the plugin config menu (press L, DPAD Down and Minus on the GamePad, Pro Controller or Classic Controller) you can configure the plugin. The available options are the following:

- **Settings**:
    - Plugin enabled: (Default is true)
        - Enables or disables the SwipSwapMe plugin. **Forces the audio output to stereo when enabled.**
    - Show notifications: (Default is true)
        - Displays notifications when swapping the screens or changing the audio mode.
    - Swap screens: (Default is false)
        - Swaps the TV and GamePad screen when set to true.
    - Screen mode: (Default is "Normal")
        - Sets the screen mode. See "Screen modes" for more details.
    - Audio mode: (Default is "Sound matches screen")
        - Sets the audio mode. See "Audio modes" for more details.
- **Button combos**
    - Enable swap screen button combo: (Default is true)
        - Determines if the screen can be swapped with a button combo.
    - Swap screen: (Default is the "TV" button)
        - Button combo to swap the TV and GamePad screen.
    - Enable swap screen button combo: (Default is false)
        - Determines if the screen can be changed with a button combo.
    - Change screen: (Default is "right stick button"/R3)
        - Button combo to change screen mode.
    - Enable change audio mode button combo: (Default is false)
        - Determines if the audio mode can be changed via a button combo.
    - Change audio: (Default is "left stick button"/L3)
        - Button combo to change the audio mode.

#### Audio Modes

SwipSwapme does not only allow you to swap the screen, it also offers multiple audio modes:

- **Normal**
    - The audio output is not touched at all.
- **Swap TV and GamePad sound**
    - Swaps the audio output of the TV and GamePad regardless of the screen swapping.
- **Sound matches screen**
    - Swaps the audio output of the TV and GamePad if the screen are swapped as well.
- **Combine TV and GamePad sound**
    - Outputs the TV and GamePad sound on both.
- **Left: TV; Right: GamePad**
    - Outputs the TV sound on the left speaker, and the GamePad sound on the right speaker.

#### Screen Modes

SwipSwapme does not only allow you to swap the screen, it also offers multiple screen modes:

- **Normal**
    - The screen output is not touched at all.
- **Swap TV and GamePad**
    - Swaps the TV and GamePad screen.
- **Mirror TV**
    - Mirrors the TV screen onto the GamePad.
- **Mirror GamePad**
    - Mirrors the GamePad screen onto the TV.

## Building

For building you need:

- [wups](https://github.com/wiiu-env/WiiUPluginSystem)
- [wut](https://github.com/devkitPro/wut)
- [libnotifications](https://github.com/wiiu-env/libnotifications)
- [libmappedmemory](https://github.com/wiiu-env/libmappedmemory)

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once
docker build . -t swipswapme-builder

# make 
docker run -it --rm -v ${PWD}:/project swipswapme-builder make

# make clean
docker run -it --rm -v ${PWD}:/project swipswapme-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src ghcr.io/wiiu-env/clang-format:13.0.0-2 -r ./src -i`
