#!/bin/bash
#Short script to test something



POLLEN=$(cat foo.json | ./JSON.sh -b |grep \"periods\",1,\"Triggers\" |grep Name |awk '{print $2}')

#Strip "
#POLLEN=${POLLEN//\"/}
POLTYPE=""

while read -r POLLEN; do
    #echo "... $POLLEN ..."
    POLTYPE+=$POLLEN
    POLTYPE+=" "
done <<< "$POLLEN"

echo $POLTYPE