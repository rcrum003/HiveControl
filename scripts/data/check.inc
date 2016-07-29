
#!/bin/bash
#foo

# Check script to see if we can pass the variable name to a function.

function check() {
    #Function to check if we have a real value
    #Useful when we pull values from multiple sources
    #Usage check variablename (omit the $)
    if [[ ${!1} == "null" ]]; then
      return
    fi 
      #echo "value of [${1}] is: [${!1}]"
      #Regex that allows for postive and decimal numbers
      re='^[0-9]+([.][0-9]+)?$'
     	if ! [[ ${!1} =~ $re ]] ; then
     		#If we fail this check, we set the value to null so we don't screw up our charts.
     		let ${1}='null'
     	fi
    #echo "2value of [${1}] is: [${!1}]"
}	

function check_allow_neg() {
    #Function to check if we have a real value
    #Useful when we pull values from multiple sources
    #Usage check variablename (omit the $)
    
    if [[ ${!1} == "null" ]]; then
      return
    fi 

    #echo "value of [${1}] is: [${!1}]"
    #Regex that allows for postive and decimal numbers
    re='^-?[0-9]+([.][0-9]+)?$'
    if ! [[ ${!1} =~ $re ]] ; then
      #If we fail this check, we set the value to zero so we don't screw up our charts.
      let ${1}='null'
    fi
}
