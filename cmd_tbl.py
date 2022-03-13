#!/usr/local/bin/python
# -*- coding: utf-8 -*-
################################################################################
# cmd_tbl.py
# Description:  
#
# Author:       shozo fukuda
# Date:         Sat Mar 12 07:38:18 JST 2022
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
class CmdTbl:
    def __init__(self, prefix="", namespace=""):
        self.prefix = prefix
        self.ns     = namespace
        self.func   = []
        self.col    = 40

    def parse(self, file):
        name  = None
        for line in file:
            match = re.search(r'\bCIMG_CMD\s*\((.*)\)', line)
            if match:
                name = match.group(1)
                self.func.append(name)

    def mk_cmdtbl(self, output):
        for name in self.func:
            idx_name = self.prefix + name
            cxx_name = self.ns + name
            print('{{"{idx_name}",{pad:{loc1}}{cxx_name}{pad:{loc2}}}},'
                   .format(
                       idx_name=idx_name,
                       loc1=self.col-len(idx_name)-3,
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
    parser = argparse.ArgumentParser(description="Generate CImg command table")
    parser.add_argument('src', type=argparse.FileType('r'),
        help="src files")
    parser.add_argument('--prefix', default="",
        help="prefex for export name")
    parser.add_argument('--ns', default="",
        help="namespace for entry function")
    parser.add_argument('-o', '--output', type=argparse.FileType('w'), default=sys.stdout,
        help="output file")
    args = parser.parse_args()

    cmd_tbl = CmdTbl(args.prefix, args.ns)
    cmd_tbl.parse(args.src)
    cmd_tbl.mk_cmdtbl(args.output)

# cmd_tbl.py
