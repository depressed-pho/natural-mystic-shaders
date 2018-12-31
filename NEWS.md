# Release notes

## 1.2.0

Enhancements:

* Added fake specular light for water and improved its wave. The
  opacity of water surface now changes depending on the angle between
  the camera and the surface. There is still a room for improvement in
  the specular light though. (#10)

Bugfixes:

* Lava pools in the Nether no longer look strangely whitish. (#30)
* Performance improvement for clouds.
* Fixed the color and the intensity of ambient light being calculated
  incorrectly.
* Shadows no longer disappear completely when it rains. It still fades
  suddenly and we're looking for a way to fix it.
* The animation speed of leaves no longer accelerate when the camera
  is moving. Water still has the same issue but we don't know how to
  fix it.

## 1.1.0

Enhancements:

* The scene now looks duller under a bad weather condition. (#6)
* Added waves for water and leaves, but not plants due to a limitation
  in the current rendering system. (#10, #11)
* Replaced clouds with non-blocky, shader-generated one.

## 1.0

The first release.
