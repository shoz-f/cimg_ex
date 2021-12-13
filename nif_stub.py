#!/usr/local/bin/python
# -*- coding: utf-8 -*-
################################################################################
# nif_stub.py
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
class NifStub:
    def __init__(self, module):
        self.module = module
        self.app    = ':'+module.split('.')[0].lower()
        self.nif    = module.lower().replace('.', '_')      # restrict nif name
        self.func   = []

    def parse(self, file):
        for line in file:
            match = re.search(r'^{"(.*)",\s*(\d+),', line)
            if match:
                name  = match.group(1)
                ality = int(match.group(2))
                self.func.append((name, ality))

    def mk_niftbl(self, output):
        print('defmodule {module} do\n'
               '  @moduledoc false\n'
               '\n'
               '  #loading NIF library\n'
               '  @on_load :load_nif\n'
               '  def load_nif do\n'
               '    nif_file = Application.app_dir({app}, "priv/{nif}")\n'
               '    :erlang.load_nif(nif_file, 0)\n'
               '  end\n'
               '\n'
               '  # stub implementations for NIFs (fallback)\n'
               .format(
                   module=self.module,
                   app=self.app,
                   nif=self.nif),
               end='',
               file=output)

        for name, ality in self.func:
            print('  def {name}({prms}),\n    do: raise("NIF {name}/{ality} not implemented")'
                   .format(
                       name=name,
                       ality=ality,
                       prms=', '.join(["_%d"%(x+1) for x in range(ality)])),
                   file=output)

        print('end', file=output)

#<MAIN>#########################################################################
# Function:     
# Description:  
# Dependencies: 
################################################################################
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate NIFs table")
    parser.add_argument('module',
        help="module name")
    parser.add_argument('srcs', nargs='+', type=argparse.FileType('r'),
        help="src files")
    parser.add_argument('-o', '--output', type=argparse.FileType('w'), default=sys.stdout,
        help="output file")
    args = parser.parse_args()

    nif_stub = NifStub(args.module)
    for src in args.srcs:
        nif_stub.parse(src)
    nif_stub.mk_niftbl(args.output)

# nif_stub.py
