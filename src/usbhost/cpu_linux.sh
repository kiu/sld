#!/bin/bash
# by Paul Colby (http://colby.id.au), no rights reserved ;)
 
PREV_TOTAL=0
PREV_IDLE=0
 
while true; do
  CPU=(`cat /proc/stat | grep '^cpu '`) # Get the total CPU statistics.
  unset CPU[0]                          # Discard the "cpu" prefix.
  IDLE=${CPU[4]}                        # Get the idle CPU time.
 
  # Calculate the total CPU time.
  TOTAL=0
  for VALUE in "${CPU[@]}"; do
    let "TOTAL=$TOTAL+$VALUE"
  done
 
  # Calculate the CPU usage since we last checked.
  let "DIFF_IDLE=$IDLE-$PREV_IDLE"
  let "DIFF_TOTAL=$TOTAL-$PREV_TOTAL"
  let "DIFF_USAGE=(1000*($DIFF_TOTAL-$DIFF_IDLE)/$DIFF_TOTAL+5)/10"

  # Remember the total and idle CPU times for the next check.
  PREV_TOTAL="$TOTAL"
  PREV_IDLE="$IDLE"

  LED_ON=$(($DIFF_USAGE / 8))
  CMD=""
  for (( c=0; c<$LED_ON; c++ ))
  do
    CMD+="\x03"
  done
  for (( c=$LED_ON; c<13; c++ ))
  do
    CMD+="\x00"
  done

  echo "CPU: $DIFF_USAGE%"
  echo -n -e $CMD | ./sldtool_linux > /dev/null
 
  # Wait before checking again.
  sleep 1
done
