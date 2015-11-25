#!/bin/bash

set -e
ctr-logobuilder --dirpath=$2 --bottomtex=hblogo_bottom.bclim --toptex=hblogo_top.bclim "--bottomtexcoords=$3" "--toptexcoords=$4"
darctool --build $1.bin $2
openssl sha -sha256 -mac HMAC -macopt hexkey:`cat $HOME/.3ds/logo_hmackey_text` $1.bin | cut -f 2 -d " " | xxd -r -p >> $1.bin
bannertool lz11 -i $1.bin -o $1.lz11
dd if=/dev/zero of=$1-padded.lz11 bs=1 count=8192
dd conv=notrunc if=$1.lz11 of=$1-padded.lz11

