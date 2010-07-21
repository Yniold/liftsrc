#!/bin/bash
echo C3F6 switching script is running

while [ 1 ]
do 
echo "switch on valve"

# read ValveWord 
let ValveWord=$(/lift/bin/eCmd @armAxis r 0xa408 | tail -1) 

# 4 test only:
echo "The ValveWord was $ValveWord before..."


# calculate new ValveWord
let NewValveWord=$(($ValveWord | 8192))  

# 4 test only:
echo "...now the new ValveWord is $NewValveWord!" 

/lift/bin/eCmd @armAxis w 0xa460 3360 # 24V needed to switch solenoids on
/lift/bin/eCmd @armAxis w 0xa408  $NewValveWord
/lift/bin/eCmd @armAxis w 0xa460 2100 # 15V needed to keep Pitot Zero open

echo "set MFC to 50 sccm"
/lift/bin/eCmd @armAxis w 0xa440 26

sleep 30

echo "switch off valve"

# read ValveWord 
ValveWord=$(/lift/bin/eCmd @armAxis r 0xa408 | tail -1) 

# 4 test only:
echo "The ValveWord was $ValveWord before..."


# calculate new ValveWord
NewValveWord=$(($ValveWord & 57343))  

# 4 test only:
echo "...now the new ValveWord is $NewValveWord!" 

/lift/bin/eCmd @armAxis w 0xa460 3360 # 24V needed to switch solenoids on
/lift/bin/eCmd @armAxis w 0xa408  $NewValveWord
/lift/bin/eCmd @armAxis w 0xa460 2100 # 15V needed to keep Pitot Zero open

echo "set MFC to 0 sccm"
/lift/bin/eCmd @armAxis w 0xa44 0

sleep 30


done
