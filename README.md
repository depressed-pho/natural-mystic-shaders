# Natural Mystic Shaders

**Natural Mystic Shaders** is a shader pack for Minecraft Bedrock
aiming to be as realistic as possible.

![Screenshot during daytime](./img/day.jpg)
![Screenshot during night-time](./img/night.jpg)

Features include:

* Shadows
* Non-blocky clouds
* Waves for water and leaves (but not plants)
* Camera-angle dependent transparency and specular light for water
* Improved torch light with flickering effect (unstable in 1.8 due to vanilla bug)
* Sunlight and moonlight colors with night time desaturation
* Desaturation under a bad weather condition
* Exposure adjustment, contrast filter, and tone mapping

## Download

See [releases](https://github.com/depressed-pho/natural-mystic-shaders/releases).

## Release notes

See [NEWS](NEWS.md).

## Supported platforms

* iOS
* Android (untested)

## Tested on

* Minecraft Bedrock 1.8, iPad Pro (MLMY2J/A), iOS 12.1 (16B92)

## Changing configuration

Ideally there should be an in-game GUI to dynamically control the
settings of shaders, but that isn't possible at the moment (#44). To
compensate this shader pack contains a configuration file so users can
edit it directly to modify its behavior.

* Unpack the `.mcpack` file to some directory. It's just a zip archive.
* Open `src/shaders/glsl/natural-mystic-config.h` with your favorite editor.
* Edit the file to change configuration.
* Archive it again, and install it.

For example, you can disable the shader-generated clouds and turn it
into the default by changing the following line:

```glsl
#define ENABLE_FBM_CLOUDS 1
```

To this:

```glsl
//#define ENABLE_FBM_CLOUDS 1
```

## Author

PHO

## License
[CC0](https://creativecommons.org/share-your-work/public-domain/cc0/)
“No Rights Reserved”
