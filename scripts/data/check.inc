
#!/bin/bash

# Check script to see if we can pass the variable name to a function.

function check() {
    #Function to check if we have a real value
    #Useful when we pull values from multiple sources
    #Usage check variablename (omit the $)
    
    #echo "value of [${1}] is: [${!1}]"
    #Regex that allows for postive and decimal numbers
    re='^[0-9]+([.][0-9]+)?$'
   	if ! [[ ${!1} =~ $re ]] ; then
   		#If we fail this check, we set the value to zero so we don't screw up our charts.
   		let ${1}=0
   	fi
}	

function check_allow_neg() {
    #Function to check if we have a real value
    #Useful when we pull values from multiple sources
    #Usage check variablename (omit the $)
    
    #echo "value of [${1}] is: [${!1}]"
    #Regex that allows for postive and decimal numbers
    re='^-?[0-9]+([.][0-9]+)?$'
    if ! [[ ${!1} =~ $re ]] ; then
      #If we fail this check, we set the value to zero so we don't screw up our charts.
      let ${1}=0
    fi
}