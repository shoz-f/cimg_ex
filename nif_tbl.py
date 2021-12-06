#!/usr/local/bin/python
# -*- coding: utf-8 -*-
################################################################################
# nif_tbl.py
# Description:  
#
# Author:       shozo fukuda
# Date:         Mon Dec  6 07:36:25 2021
# Last revised: $Date$
# Application:  Python 3.9
################################################################################

#<IMPORT>
import os,sys
import argparse
import re

#<CLASS>########################################################################
# Description:  
# Dependencies: 
################################################################################
class NifTbl:
    def __init__(self, prefix="", namespace=""):
        self.prefix = prefix
        self.ns     = namespace
        self.keys   = []
        self.ality  = dict()

    def parse(self, file):
        key  = ""
        for line in file:
            match = re.search(r'\bDECL_NIF\s*\((.*)\)', line)
            if match:
                key = match.group(1)
                continue

            match = re.search(r'ality\s*!=\s*(\d+)', line)
            if match:
                self.ality[key] = int(match.group(1))
                self.keys.append(key)
                continue

    def mk_niftbl(self, output):
        for key in self.keys:
            name  = self.prefix + key
            ality = self.ality[key]
            entry = self.ns + key
            text  = '{{"{name}",{pad:{loc1}}{ality:2d}, {entry},{pad:{loc2}}0}},'.format(
                name=name,
                loc1=38-len(name),
                ality=ality,
                entry=entry,
                loc2=38-len(entry),
                pad='')
            print(text, file=output)

#<SUBROUTINE>###################################################################
# Function:     
# Description:  
# Dependencies: 
################################################################################
def make_nifstub(file, keys, ality, prefix1=None, prefix2=None):
    for key in keys:
        name  = (prefix1 or "")+key
        
        text = 'def {name}(_1),\n'\
               '  do: raise("NIF {name}/{ality} not implemented")'.format(
                 name=name,
                 ality=ality[key],
               )
        print(text)

#<MAIN>#########################################################################
# Function:     
# Description:  
# Dependencies: 
################################################################################
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate NIFs table")
    parser.add_argument('src', type=argparse.FileType('r'),
        help="src files")
    parser.add_argument('--prefix', default="",
        help="prefex for export name")
    parser.add_argument('--ns', default="",
        help="namespace for entry function")
    parser.add_argument('-o', '--output', type=argparse.FileType('w'), default=sys.stdout,
        help="output file")
    args = parser.parse_args()

    nif_tbl = NifTbl(args.prefix, args.ns)
    nif_tbl.parse(args.src)
    nif_tbl.mk_niftbl(args.output)

# nif_tbl.py
