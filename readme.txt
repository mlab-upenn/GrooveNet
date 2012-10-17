The GrooveNet Simulator has been tested on Ubuntu 12.04. This is qt3 OSM map beta version.  

1. First, make sure you have installed qt3 platform and g++ compiler. It works fine for g++ with 4.4.5 or above.
   
   - to install qt3:
     type 'sudo apt-get install qt3-dev-tools' in the terminal
   
   - to install g++:
     type 'sudo apt-get install g++' in the terminal

2. Then please go to https://github.com/mlab/GrooveNet

3. Please click 'Download' to download the GrooveNet simulator.

4. Navigate to your download folder and untar (or unzip) the package. You may use tar -xzvf 'file name'

5. cd into your extracted folder -> project

6. type 'make clean'

7. type 'make' to build and install GrooveNet.

8. If there are no errors, cd into bin folder

9. type './groovenet' to run the simulator


Map import:
Rename your OSM map file according to ../mapdata/counties.txt
For example, you've downloaded the map of Philadelphia county, you will need to rename it to "42101.osm" before you can use it.


TODO:

1. In the current version, you can only find a address by using intersection(eg. 34th St & Walnut St, Philadelphia, PA), you can NOT use normal address(eg. 3401 Walnut St, Philadelphia) because OSM map does not provide address range info, which is essential for generating normal addresses. You can fix this by either import address range info or generate address range by estimation.

2. Add random address generation based on intersection address. Currently, random address generation is based on normal address, all function related to random address generation will not work properly. 

3. Generate water polygon.
(If TODO1 is implemented, TODO2 can be ignored)
