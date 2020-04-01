
#!/bin/bash

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

function check_if_blank() {
    #Function to check if we have a real value
    #Useful when we pull values from multiple sources
    #Usage check variablename (omit the $)
    
    if [[ ${!1} == "" ]]; then
      let ${1}='null'
    fi 

}

function resonable_check() {
    #Function to check if we have a real value, as opposed to some sensor going crazy and returning a crazy value
    
    # Usage
    # resonable_check type new_value

    #Set some standard values:
    LOCALDATABASE="/home/HiveControl/data/hive-data.db"
    datatype = "$1"
    new_value = "$2"

    #Get old_value depending on the type
    old_value=$(sqlite3 $LOCALDATABASE "SELECT $datatype from allhivedata where date >= Datetime('now','-5 minutes', 'localtime')"
    
    #If last value was null, then let's use this value if it's resonable
    
    case "$datatype" in
      hivetempf )
        #Highs shouldn't exceed 140F

        #Lows shouldn't exceed -50F

        #shouldn't drop or rise more than 10 degrees in 5 minutes

        ;;
      humidity )
        #should be between 1 and 100

        #shouldn't drop or rise more than 50 in 5 minutes
        ;;

      hiveweight )
        #shouldn't be negative

        #shouldn't exceed 500 lbs

        #shouldn't change more than the average weight of a super, unless we are taking off a super, in which case it should

        ;;
    esac

    #Compare to new_value

    #Return positive or negative

    
    #echo "value of [${1}] is: [${!1}]"
    #Regex that allows for postive and decimal numbers
    #re='^-?[0-9]+([.][0-9]+)?$'
    #if ! [[ ${!1} =~ $re ]] ; then
      #If we fail this check, we set the value to zero so we don't screw up our charts.
      #let ${1}='0'
    #fi
}