#!/bin/sh
# @Author: anchen
# @Date:   2017-01-05 16:57:50
# @Last Modified by:   anchen
# @Last Modified time: 2017-01-08 20:02:52
cdr_decode_save_path=$1
back_str=/
cdr_path=$2
decode_data_path=/home/yangwc/url_test/decode/data/
rm_decode_data_path=/home/yangwc/url_test/decode/data/* 
decode_parm_path=/home/yangwc/url_test/decode/
decode_data_out_path=/home/yangwc/url_test/decode/output/*
parm_home=/home/yangwc/url_test/
cp $cdr_path ./tmp/ 
tar -jxvf ./tmp/* 
rm ./tmp/nti_cdr*
mv nti_cdr* $decode_data_path
cd $decode_parm_path
. env.sh
./fun 25 13 
rm $rm_decode_data_path
cd $parm_home
mv $decode_data_out_path .
mkdir $cdr_decode_save_path
mv suspicion_phone_trace* $cdr_decode_save_path$back_str
python re_cmp.py $cdr_decode_save_path$back_str