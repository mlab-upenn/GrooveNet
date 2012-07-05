# Created by Daniel Weller
# Network shutdown for the GrooveSim Hybrid Simulator
# Tell GrooveSim to use this file for network shutdown
# and it will execute it for you whenever the network needs
# to be shutdown.

# This script is called as follows:
# ./netclose.sh <device name> <device type> <IP address> <subnet mask> <tx power> <tx rate>
#

if [ "$#" -ne 6 ]
then
  echo "Usage: the netclose.sh script is called as follows:"
  echo "  ./netclose.sh <device name> <device type> <IP address> <subnet mask> <tx power> <tx rate>"
  echo
  echo "Example: ./netclose.sh eth0 denso_waveapi 192.168.0.136 255.255.0.0 20 0"
  exit 1
fi

# Commands to execute
IFCONFIG_CMD=ifconfig
INSMOD_CMD=insmod
RMMOD_CMD=rmmod
ROUTE_CMD=route
WAVEINIT_CMD=./waveinit

# Predefined constants
VAR_ADHOCBAND=1
VAR_ANTENNA=0
VAR_BSSID='"393837363534"'
VAR_BSSTYPE=2
VAR_CLIST='"5800"'
VAR_DRIVER=ar5211-sta-x86
VAR_DRIVER_PATH=/lib/modules/2.4.18-3/kernel/drivers/net/wireless/ar5211-sta-x86.o
VAR_IPADDR_ROUTE=255.255.255.255
VAR_RATECTRL=0
VAR_SME=0
VAR_SSID='"dsrc"'
VAR_SUBNET_ROUTE=255.255.255.255

# Get arguments
ARG_DEVICE_NAME=$1
ARG_DEVICE_TYPE=$2
ARG_IPADDR_LOCAL=$3
ARG_SUBNET_LOCAL=$4
ARG_FORCEPOWER=$5
ARG_TXRATE11A=$6

###############################################################################
#
# If you want to use AODV, make sure to bring down the aodvd daemon!
#
# AODV_KILL_CMD="killall -q -w aodvd"
# echo $AODV_KILL_CMD
# $AODV_KILL_CMD
#
###############################################################################

# Bring down the interface
IFCONFIG_DOWN_CMD="$IFCONFIG_CMD $ARG_DEVICE_NAME down"
echo $IFCONFIG_DOWN_CMD
$IFCONFIG_DOWN_CMD
ERRORCODE="$?"
if [ $ERRORCODE -ne 0 ]
then
  echo "Error: Could not shut down this device! (code $ERRORCODE)"
  exit 1
fi

# Remove the module from use
# REMOVE_MOD_CMD="$RMMOD_CMD $VAR_DRIVER"
# echo $REMOVE_MOD_CMD
# $REMOVE_MOD_CMD
# ERRORCODE="$?"
# if [ $ERRORCODE -ne 0 ]
# then
#   echo "Error: Could not remove this module! (code $ERRORCODE)"
#   exit 1
# fi

echo "Shutdown completed successfully!"

# End script
