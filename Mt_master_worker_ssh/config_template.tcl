#
# This is a template file to configure the director/master/worker
# system. The configuration is performed with the help of two commands,
# "add_project" and "add_workload". The command syntax is given below:
#
# add_project id tclfiles analysis_command cfiles? flags?
#
# This command adds another project. Projects consist of the collection
# of code files, tcl command to run on each ntuple, and compiler switches.
# Arguments are as follows:
#
#   id                  This is the project id -- an arbitrary string.
#                       All projects must have distinct ids. A special
#                       keyword "auto" can be used in place of the id
#                       argument to generate a new unique project id
#                       automatically.
#
#   tclfiles            The list of tcl files which contain all necessary
#                       code evaluated before the ntuple processing is
#                       commenced. The files must be on the local machine.
#                       The code will be concatenated and sent to the
#                       worker nodes for evaluation.
#
#   analysis_command    The command to run on each ntuple. The system
#                       will append the ntuple id as the last argument.
#                       To the user code, the ntuple to process will
#                       appear as the ntuple with the same header as
#                       the workload ntuple, but with one row only.
#                       This command must process the data, create a new
#                       ntuple to store the result, and return the id
#                       of the new ntuple.
#
#   cfiles              Optional list of C files which contain the analysis
#                       code. The code in the files will be concatenated
#                       locally, shipped to the worker machine, and compiled
#                       there.
#
#   flags               Optional compilation and linking flags to give
#                       the hs::sharedlib_compile command. For example,
#                       "-cflags {-O1 -ggdb} -fflags {-O} -linkflags {-lm}".
#
# If everything is fine, the id of the new project is returned.
#
# To add data files to a project, use the following command:
#
# add_workload project_id host_patterns input_file ntuple_title \
#              ntuple_category result_category output_file
#
# This command adds a Histo-Scope ntuple to the project. Arguments are:
#
#   project_id          Project id returned by the "add_project" command.
#
#   host_patterns       {A list of patterns which will be matched against
#                       the names of the worker nodes. The work load will
#                       be processed on the worker node only if one of
#                       the patterns in this list matches the worker host
#                       name.} THIS ARGUMENT IS CURRENTLY IGNORED.
#
#   input_file          The location of the input Histo-Scope file. If
#                       the location specifier starts with "prefix:" then
#                       it may be a remote file. In this case an attempt
#                       will be made (by a master node) to transfer the file
#                       to a local temporary directory. The following
#                       prefixes are recognized:
#
#                        ftp, http    The location must be a valid URL.
#                                     "wget" will be used to download
#                                     the file into a local directory.
#
#                        scp, rcp     The location specifies a file
#                                     accessible via scp or rcp. The
#                                     remainder of the location string
#                                     must specify the host and file name
#                                     in the usual scp/rcp format. E.g.,
#                                     scp:cdflx4.lbl.gov:/home/igv/file.hs
#
#                        file         The remainder is the file name
#                                     on the local machine.
#
#                       If the prefix is not specified or not recognized,
#                       the whole name is treated as a file name on the
#                       local machine.
#
#   ntuple_title        The title of the input ntuple.
#
#   ntuple_category     The category of the input ntuple.
#
#   result_category     The category where result ntuples will be placed.
#                       At the end, there will be one result ntuple per
#                       each row of the input ntuple. The user ids of
#                       the result ntuples will be set to the corresponding
#                       row numbers of the input ntuple. Input ntuple
#                       category and result category must be different.
#
#   output_file         The location of the file where the result ntuples
#                       will be saved. The output file will be overwritten
#                       if it exists. Prefixes "scp", "rcp", and "file"
#                       can be used with the same meaning as for the input
#                       file. "ftp" and "http" protocols are not supported
#                       for output files.
#
# This command returns an automatically assigned id of the "workload" object.
# The workload id can later be used to get info about this workload from
# the monitoring socket.
#
# Note that the project configuration file is evaluated by a safe interpreter.
# Some tcl commands are disabled in this interpreter. In particular, the "file"
# command is disabled.
