# usage curl_get $url
# include in script file
# Usage: first include - source /scripts/http/curl.inc
# In script - curl_get $USERNAME $SRC_URL $TEMPFILE $JOB_NAME
# Note - If using authentication, you must add to the ~/.netrc file
# version 2019061001


function curl_get {

# Gets a webpage you specify
# This script takes 4 inputs 
# 1 - Source URL 
# 2 - Output file name
# 3 - JobName
# 4 - Do you want to run NTLM, pass NTLM as the fourth variabl

SRC_URL="$1"
TEMPFILE="$2"
JOB_NAME="$3"
NTLM="$4"

if [[ $NTLM = "NTLM" ]]; then
    #statements
    curl_status=$(curl -s -n -w %{http_code} --ntlm "$SRC_URL" -o $TEMPFILE)
else 
    curl_status=$(curl -s -n -w %{http_code} "$SRC_URL" -o $TEMPFILE)
fi

#echo "CurlStatus is $curl_status"
#Check that we ran:

check_curl_status $curl_status

}


function check_curl_status {

#Added Exit mode, so if we have scripts that need to continue to run, you can specify that in the check_status

# 1 - The Curl Status Code
# 2 - Exit mode - Continue or Fail
# 3 - The URL we tried
# 4 - Retry - Pass the word Retry as 4th character and it'll retry


curl_status="$1"
exit_mode="$2"
SRC_URL="$3"
retry_mode="$4" #Set to "Retry" in curl script (case sensitive) to invoke
JOBNAME="$5"

RETURN_CURL_STATUS="0"
unset ERROR

case "$curl_status" in
        000)
            #Connection Error             
            if [[ $retry_mode == "Retry" ]]; then
                RETURN_CURL_STATUS="3"
            else
                ERROR="Connection Error - Check URL - We tried $SRC_URL"
                RETURN_CURL_STATUS="1"
            fi
            ;;
        200)
            #loglocal "$DATE" $JOB_NAME INFO "Successfully downloaded file with curl"
            #RETURN_CURL_STATUS="0"
            if [[ $retry_mode == "Retry" ]]; then
                RETURN_CURL_STATUS="0"
                #Only communicate status if the script calls for Retry
                echo "Success"
            else
                RETURN_CURL_STATUS="0"
            fi
            ;;
        [3]*)
            ERROR="Received a redirection response of - $curl_status for URL ($SRC_URL)"
            RETURN_CURL_STATUS="1"
            ;;
        401)
            ERROR="401 Unauthorized - Authentication failed for user in .netrc"
            RETURN_CURL_STATUS="1"
            ;;  
        403)
            ERROR="403 Forbidden - Permissions error for user"
            RETURN_CURL_STATUS="1"
            ;; 
        404)
            ERROR="404 Not Found - SRC_URL tried =  $SRC_URL"
            RETURN_CURL_STATUS="1"
            ;;
        500)
            ERROR="500 Internal Server Error - SRC_URL tried =  $SRC_URL"
            RETURN_CURL_STATUS="1"
            ;;              
        null)
            loglocal "$DATE" $JOB_NAME ERROR "Unknown Error of $curl_status"
            RETURN_CURL_STATUS="1"
            ;;
        *)
        #Didn't bother to define all the HTTP responses, just the ones that we care about.
        ERROR="Unhandled HTTP RESPONSE CODE - Server said - $curl_status"
        RETURN_CURL_STATUS="1"
esac


if [[ "$RETURN_CURL_STATUS" == "1" ]]; then
    #We had an error, so exit out of whatever script has called us.
    #echo "$ERROR"
    
    if [[ $exit_mode == "Continue" ]]; then
        # Don't exit, because they said continue
        echo "Curl Error of $ERROR, but we are continuing"
    else
        #Any other status besides continue, then exit like we originally coded for
        echo "Curl Error - $Error"
        loglocal "$DATE" $JOB_NAME ERROR "$ERROR"
        exit 1
    fi
elif [[ "$RETURN_CURL_STATUS" == "3" ]]; then
    #Added a retry function
    loglocal "$DATE" $JOB_NAME INFO "Curl Error - Retrying"
    echo "RETRY"

fi

}