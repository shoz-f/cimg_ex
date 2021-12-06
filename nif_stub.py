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
    def __init__(self, prefix="", namespace=""):
        self.prefix = prefix
        self.ns     = namespace
        self.keys   = []
        self.ality  = dict()

    def parse(self, file):
        key  = ""
        for line in file:
            match = re.search(r'^{"(.*)",\s*(\d+),', line)
            if match:
                key = match.group(1)
                self.ality[key] = int(match.group(2))
                self.keys.append(key)

    def mk_niftbl(self, output):
        print('defmodule do\n'
               '  @moduledoc false\n'
               '\n'
               '  #loading NIF library\n'
               '  @on_load :load_nif\n'
               '  def load_nif do\n'
               '    nif_file = Application.app_dir(:cimg, "priv/cimg_nif")\n'
               '    :erlang.load_nif(nif_file, 0)\n'
               '  end\n'
               '\n'
               '  # stub implementations for NIFs (fallback)',
               end='',
               file=output)
        
        for key in self.keys:
            name  = self.prefix + key
            ality = self.ality[key]
            prms  = ', '.join(["_%d"%(x+1) for x in range(ality)])
            text = '  def {name}({prms}),\n    do: raise("NIF {name}/{ality} not implemented")'.format(
                name=name,
                ality=ality,
                prms=prms)
            print(text, file=output)

        print('end', file=output)

#<MAIN>#########################################################################
# Function:     
# Description:  
# Dependencies: 
################################################################################
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate NIFs table")
    parser.add_argument('srcs', nargs='+', type=argparse.FileType('r'),
        help="src files")
    parser.add_argument('--prefix', default="",
        help="prefex for export name")
    parser.add_argument('--ns', default="",
        help="namespace for entry function")
    parser.add_argument('-o', '--output', type=argparse.FileType('w'), default=sys.stdout,
        help="output file")
    args = parser.parse_args()

    nif_stub = NifStub(args.prefix, args.ns)
    for src in args.srcs:
        nif_stub.parse(src)
    nif_stub.mk_niftbl(args.output)

# nif_stub.py
