set compilers_dir = /cdf/proj/401.top/igv/compilers
setenv BASEDIR /cdf/home/$USER/Topmass_SL6
setenv HISTO_DIR $BASEDIR
setenv PATH $BASEDIR/bin:$compilers_dir/bin:$PATH

printenv LD_LIBRARY_PATH >& /dev/null
if ($status) then
    setenv LD_LIBRARY_PATH $BASEDIR/lib:$compilers_dir/lib64:/usr/lib64:/lib64
else
    setenv LD_LIBRARY_PATH $BASEDIR/lib:$compilers_dir/lib64:/usr/lib64:/lib64:$LD_LIBRARY_PATH
endif
setenv LD_LIBRARY_PATH $BASEDIR/pythia8/lib:$BASEDIR/JetUser:$BASEDIR/W4jets_madgraph:$LD_LIBRARY_PATH
