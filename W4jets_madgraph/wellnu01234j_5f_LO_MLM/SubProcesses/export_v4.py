################################################################################
#
# Copyright (c) 2009 The MadGraph5_aMC@NLO Development team and Contributors
#
# This file is a part of the MadGraph5_aMC@NLO project, an application which 
# automatically generates Feynman diagrams and matrix elements for arbitrary
# high-energy processes in the Standard Model and beyond.
#
# It is subject to the MadGraph5_aMC@NLO license which should accompany this 
# distribution.
#
# For more information, visit madgraph.phys.ucl.ac.be and amcatnlo.web.cern.ch
#
################################################################################
"""Methods and classes to export matrix elements to v4 format."""

import copy
from cStringIO import StringIO
from distutils import dir_util
import itertools
import fractions
import glob
import logging
import math
import os
import re
import shutil
import subprocess
import sys
import getopt
import fileinput

pjoin = os.path.join

_file_path = os.path.split(os.path.dirname(os.path.realpath(__file__)))[0] + '/'
 
#===============================================================================
# ProcessExporterFortranMEGroup
#===============================================================================
def process(arg):
    """Class to take care of exporting a set of matrix elements to
    MadEvent subprocess group format."""

    matrix_file = "matrix_madevent_group_v4.inc"

    print(arg)

#    p=re.compile('DSIG\d')
    p=re.compile('DSIG')
    m=re.compile('MATRIX\d')
    for line in fileinput.input("auto_dsig.f",inplace=True):
        t=p.findall(line)
        if t:
            for pattern in set(t):
                line=line.replace(pattern,arg+"_"+pattern)
        print line,
        
    for file in glob.glob("auto_dsig[1-9]*.f"):        
        for line in fileinput.input(file,inplace=True):
            t=p.findall(line)
            if t:
                for pattern in set(t):
                    line=line.replace(pattern,arg+"_"+pattern)
            print line,
            
    for file in glob.glob("matrix[1-9]*.f"):
        for line in fileinput.input(file,inplace=True):
            t=m.findall(line)
            if t:
                for pattern in set(t):
                    line=line.replace(pattern,arg+"_"+pattern)
            print line,
            
    for file in glob.glob("auto_dsig[1-9]*.f"):
        for line in fileinput.input(file,inplace=True):
            t=m.findall(line)
            if t:
                for pattern in set(t):
                    line=line.replace(pattern,arg+"_"+pattern)
            print line,

    sys.exit()
    
    #===========================================================================
    # write_super_auto_dsig_file
    #===========================================================================
    def write_super_auto_dsig_file(self, writer, subproc_group):
        """Write the auto_dsig.f file selecting between the subprocesses
        in subprocess group mode"""

        replace_dict = {}

        # Extract version number and date from VERSION file
        info_lines = self.get_mg5_info_lines()
        replace_dict['info_lines'] = info_lines

        matrix_elements = subproc_group.get('matrix_elements')

        # Extract process info lines
        process_lines = '\n'.join([self.get_process_info_lines(me) for me in \
                                   matrix_elements])
        replace_dict['process_lines'] = process_lines

        nexternal, ninitial = matrix_elements[0].get_nexternal_ninitial()
        replace_dict['nexternal'] = nexternal

        replace_dict['nsprocs'] = 2*len(matrix_elements)

        # Generate dsig definition line
        dsig_def_line = "DOUBLE PRECISION " + \
                        ",".join(["DSIG%d" % (iproc + 1) for iproc in \
                                  range(len(matrix_elements))])
        replace_dict["dsig_def_line"] = dsig_def_line

        # Generate dsig process lines
        call_dsig_proc_lines = []
        for iproc in range(len(matrix_elements)):
            call_dsig_proc_lines.append(\
                "IF(IPROC.EQ.%(num)d) DSIGPROC=DSIG%(num)d(P1,WGT,IMODE) ! %(proc)s" % \
                {"num": iproc + 1,
                 "proc": matrix_elements[iproc].get('processes')[0].base_string()})
        replace_dict['call_dsig_proc_lines'] = "\n".join(call_dsig_proc_lines)

        file = open(pjoin(_file_path, \
                       'iolibs/template_files/super_auto_dsig_group_v4.inc')).read()
        file = file % replace_dict

        # Write the file
        writer.writelines(file)




"""Module docstring.

This serves as a long usage message.
"""

def main(argv=None):
    if argv is None:
        argv = sys.argv
    # parse command line options
    try:
        opts, args = getopt.getopt(argv[1:], "h", ["help"])
    except getopt.error, msg:
        print msg
        print "for help use --help"
        sys.exit(2)
    # process options
    for o, a in opts:
        if o in ("-h", "--help"):
            print __doc__
            sys.exit(0)
    # process arguments
    for arg in args:
        process(arg) # process() is defined elsewhere

if __name__ == "__main__":
    main()
