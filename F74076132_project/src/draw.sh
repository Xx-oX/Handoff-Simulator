#!/bin/bash

LIST="0.2_Best 0.2_E_13 0.2_T_15 0.2_Mine 0.33_Best 0.33_E_13 0.33_T_15 0.33_Mine 0.5_Best 0.5_E_13 0.5_T_15 0.5_Mine"
for var in $LIST
do
   python3 paint.py log_${var}.csv &
done