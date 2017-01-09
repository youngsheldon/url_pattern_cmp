#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author: anchen
# @Date:   2017-01-05 19:59:31
# @Last Modified by:   anchen
# @Last Modified time: 2017-01-08 19:49:59
import os
import sys

cdr_roor_path = '/mnt/hgfs/winshare/nti/'

def GetFilePathList(rootDir):
    FilePathList=[]
    for root,dirs,files in os.walk(rootDir):
        for filespath in files:
            FilePathList.append(os.path.join(root,filespath))
    return FilePathList

with open('setting.ini','r') as f:
    for line in f:
        cdr_file_path = cdr_roor_path + line.strip()
        dir_out = cdr_file_path.split('/')[-1].split('.')[1]
        out = './cdr_pre_handle.sh ' + dir_out + ' ' + cdr_file_path
        os.system(out)