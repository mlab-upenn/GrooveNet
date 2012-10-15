The GrooveNet Simulator has been tested on Ubuntu 12.04. This is qt3 OSM map beta version. For qt4(TIGER map) version please go to https://github.com/mlab/GrooveNet.

1. First, make sure you have installed qt3 platform and g++ compiler. It works fine for g++ with 4.4.5 or above.
   
   - to install qt3:
     type 'sudo apt-get install qt3-dev-tools' in the terminal
   
   - to install g++:
     type 'sudo apt-get install g++' in the terminal

2. Then please go to https://github.com/mlab-penn/GrooveNet

3. Please click 'Download' to download the GrooveNet simulator.

4. Navigate to your download folder and untar (or unzip) the package. You may use tar -xzvf 'file name'

5. cd into your extracted folder -> project

6. type 'make clean'

7. type 'make' to build and install GrooveNet.

8. If there are no errors, cd into bin folder

9. type './groovenet' to run the simulator


Map import:
Rename your OSM map file according to ../mapdata/couties.txt


TODO:

1. In the current version, you can only find a address by using intersection since OSM map does not provide address range info. You can fix this by either import address range info or generate address range by estimation.

2. Generate water polygon.

