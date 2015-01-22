122.ver
#!/bin/sh

INC=5001
INCDONE=10000

while [ $INC -lt $INCDONE ];
do

echo $INC
cp test test$INC

INC=`expr $INC + 1`

done


