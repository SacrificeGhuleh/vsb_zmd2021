#!/bin/bash

#MOVIEPATH="/run/media/richard/My Passport/Videos/Filmy/The Lord of the Rings The Fellowship of the Ring 2001 REPACK Extended Cut 1080p UHD BluRay DD+7.1 x264-LoRD.mkv"
#MOVIEPATH="/run/media/richard/My Passport/Videos/Seriály/Grand Tour/Season 4/The.Grand.Tour.S04E02.1080p.AMZN.WEB-DL.DDP5.1.H264-EVO.mkv"
MOVIEPATH="/run/media/richard/My Passport/Videos/Filmy/Pulp Fiction.mkv"

cmake .
make

#./mosaic -rows=5 -cols=4 -width=1280 -height=1024 -in="$MOVIEPATH" -out=output-1.jpg
#./mosaic -rows=1 -cols=1 -width=1280 -height=1024 -in="$MOVIEPATH" -out=output-2.jpg
#./mosaic -rows=1 -cols=2 -width=1280 -height=1024 -in="$MOVIEPATH" -out=output-3.jpg
#./mosaic -rows=3 -cols=1 -width=1280 -height=1024 -in="$MOVIEPATH" -out=output-4.jpg
#./mosaic -rows=6 -cols=6 -width=1280 -height=1024 -in="$MOVIEPATH" -out=output-5.jpg


./mosaic 5 4 1280 1024 "$MOVIEPATH" out/output-1.jpg
./mosaic 1 1 1280 1024 "$MOVIEPATH" out/output-2.jpg
./mosaic 1 2 1280 1024 "$MOVIEPATH" out/output-3.jpg
./mosaic 3 1 1280 1024 "$MOVIEPATH" out/output-4.jpg
./mosaic 6 6 1280 1024 "$MOVIEPATH" out/output-5.jpg