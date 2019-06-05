/*******************************************************************************
*									       *
* configFile.h -- Load and save configuration files 			       *
*									       *
* Copyright (c) 1995 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* April 7, 1995								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

enum headingIndicies {
    HISTOGRAM_HEADING = 0,
    HISTOGRAM2D_HEADING,
    INDICATOR_HEADING,
    CONTROL_HEADING,
    TRIGGER_HEADING,
    NTUPLE_HEADING,
    CELL_HEADING,
    NTUPLE_EXT_HEADING,
    MULTIPLOT_HEADING,
    WINACTION_HEADING,
    COLORCELL_HEADING,
    COLORSCALE_HEADING,
    CONFIG_HEADING,
    COMMAND_HEADING,
    HISTOGRAM3D_HEADING,
    NHEADINGS
};

int ReadConfigFile(Widget parent, char *filename);
void WriteMultilineString(FILE *fp, char *string);
int ParseConfigBuffer(Widget parent, const char *buffer);
int SaveConfigFile(Widget parent, char *filename);
