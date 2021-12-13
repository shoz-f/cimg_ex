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
        self.func   = []
        self.col    = 40

    def parse(self, file):
        name  = None
        for line in file:
            match = re.search(r'\bDECL_NIF\s*\((.*)\)', line)
            if match:
                name = match.group(1)
                continue

            match = re.search(r'ality\s*!=\s*(\d+)', line)
            if match and name != None:
                ality = int(match.group(1))
                self.func.append((name, ality))
                name = None
                continue

    def mk_niftbl(self, output):
        for name, ality in self.func:
            erl_name = self.prefix + name
            cxx_name = self.ns + name
            print('{{"{erl_name}",{pad:{loc1}}{ality:2d},  {cxx_name},{pad:{loc2}}0}},'
                   .format(
                       erl_name=erl_name,
                       loc1=self.col-len(erl_name)-3,
                       ality=ality,
                       cxx_name=cxx_name,
                       loc2=self.col-len(cxx_name)-1,
                       pad=''),
                   file=output)

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
