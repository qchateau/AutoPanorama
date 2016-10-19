## TODO : ##
* mingw on linux (cross compile ?)
* linux check dependencies
* linux deployment script (.deb gen)
* disable cancel while processing

## TODO OpenCV : ##
* linear instead of affine model for exposure compensation
* slow opencl

## This build needs the opencv submodule to be built correctly : ##
* Install Eigen for faster computation
* Linux :
    * Use script in the script folder
* Win32 :
    * Build as static lib
    * Install localy in ./opencv/install/

## To deploy the windows executable : ##
* Install Inno Installer
* Make install
* Run the script "win_installer" with Inno Installer

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
