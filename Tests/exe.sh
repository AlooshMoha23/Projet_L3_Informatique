#!/bin/bash

#fermer les sites s'il existe
killall site*
kill orch*

#compiler orch.c and site.c
gcc orch.c -o orch
gcc sites.c -o site

#paramettres
IPconfiguration=$1
Portconfiguration=$2
IPmainserveur=$3
PortMainserveur=$4
choix=$5
IpSockets=$6



#Launch orchestrator
./orch $IPconfiguration $Portconfiguration $choix 20 &



#Wait for orchestrator to start
sleep 1

#Define array to store used ports
used_ports=()

#Launch clients
for i in {1..20}
do
  while true; do
    PortSocketEcoute=$(( ( RANDOM % 2000 ) + 10000 ))
    PortToMainServeur=$(( ( RANDOM % 2000 ) + 20000 ))
    if [[ " ${used_ports[@]} " =~ " ${PortSocketEcoute} " ]] || [[ " ${used_ports[@]} " =~ " ${PortToMainServeur} " ]]; then
      continue
    fi
    used_ports+=("$PortSocketEcoute" "$PortToMainServeur")
    break
  done
  echo "Checking port $PortSocketEcoute...$PortToMainServeur....$i "
  echo "configuration $IPconfiguration...$Portconfiguration....$i "
  ./site $IPconfiguration $Portconfiguration $IpSockets $PortSocketEcoute $IPmainserveur $PortMainserveur $IpSockets $PortToMainServeur $i &
  sleep 0.5
done