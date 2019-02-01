#!/bin/bash


# VOLUME=volume-widespread.h5
VOLUME=201304181130_fivan_doppler.h5

WINDOW=${WINDOW:-'25,25'}

BASENAME='doppler-demo'
SAMPLES="$BASENAME.dat"

# Comment if not desired
rm $SAMPLES

IMAGE_BG=$BASENAME-bkg.png

echo "Demonstrates doppler dealiasing"
echo "Example:"




BBOX='{where:LL_lon} {where:LL_lat}\n{where:UR_lon} {where:UR_lat}\n'
PROJ='{where:projdef}\n'

SCALE=${SCALE:-'0.001'}

echo "Detect LAT,LON bounding box and projection."
echo "Dump data in LAT,LON positions"
echo "Convert data and the bbox to metrix coords with desired scale ($SCALE)"


if [ ! -f $SAMPLES ]; then 
    cmd="rack $VOLUME  -Q DBZH -t gain=1.0 -c -o $IMAGE_BG --format '$BBOX'  --formatOut bbox.tmp --format '$PROJ'  --formatOut proj.tmp  --pDopplerDeAlias $WINDOW --sample 30,30,skipVoid=1 --format '{LON} {LAT}\t{AMVU} {AMVV} {QIND}' -o $SAMPLES  -o out-unaliased.h5"
    echo $cmd
    eval $cmd
    #cut -d\# -f1 tmp.dat > $SAMPLES
    #fgrep -v \# tmp.dat > $SAMPLES
fi

head $SAMPLES

cat bbox.tmp proj.tmp
#source ranges.tmp
#export XRANGE
#export YRANGE

# redefine...
PROJ="`cat proj.tmp` ${SCALE:+-m $SCALE} "
echo "PROJ=$PROJ"
BBOX=( `cat bbox.tmp | proj $PROJ` )
echo "BBOX=${BBOX[*]}"
XRANGE="${BBOX[0]}:${BBOX[2]}"
YRANGE="${BBOX[1]}:${BBOX[3]}"

cat $SAMPLES | proj $PROJ > tmp.dat
mv tmp.dat $SAMPLES
head $SAMPLES


export HEADSIZE=0.05,4,8

cmd="OUTFILE=$BASENAME.png XRANGE=$XRANGE YRANGE=$YRANGE gnuplot-vectors.sh $SAMPLES $IMAGE_BG"
echo $cmd
eval $cmd
