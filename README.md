## TODO : ##
* mingw on linux (cross compile ?)
* inner cut ? how ?
* linux check dependencies
* linux deployment script (.deb gen)

## TODO OpenCV : ##
* linear instead of affine model for exposure compensation
* Blender uses 16bits signed pixels : why ?

## This build needs the opencv submodule to be built correctly : ##
* Linux :
    * Use script in the script folder
* Win32 :
    * Build as static lib
    * Install localy in ./opencv/install/

## To deploy the windows executable : ##
* Copy the content of lib_for_windows
* Copy the executable

## References : ##
#### Exposure compensation on all channels : ####
Richard Szeliski. Image alignment and stitching: A tutorial. Technical Report MSR-TR-2004-92, Microsoft Research, December 2004.
page 23

#### Multi-feed for exposure compensation : ####
M. Uyttendaele, A. Eden, and R. Szeliski. Eliminating ghosting and exposure artifacts in image mosaics. In Proc. CVPR’01, volume 2, pages 509–516, 2001
page 4

#### Correct exposition before finding seams : ####
M. Uyttendaele, A. Eden, and R. Szeliski. Eliminating ghosting and exposure artifacts in image mosaics. In Proc. CVPR’01, volume 2, pages 509–516, 2001
page 4

#### Removed the blocks gains filtering (?) : ####
M. Uyttendaele, A. Eden, and R. Szeliski. Eliminating ghosting and exposure artifacts in image mosaics. In Proc. CVPR’01, volume 2, pages 509–516, 2001
Describes the optimal block size as 32 for full-sizes images. In the OpenCV implementation, blocks are calculated on
a decimated image. Therefore they are much "bigger" than 32 pixels once the gain map have been resized.
Current implementation can't use actual 32 pixels-wide blocks due to memory complexity reasons.
To compensate for the block size being bigger, I removed gains filtering. This gives better results.
