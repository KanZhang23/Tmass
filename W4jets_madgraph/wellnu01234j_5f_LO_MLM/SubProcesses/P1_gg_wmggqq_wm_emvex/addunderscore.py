import os,sys,glob

file_list = glob.glob("matrix*.f")
file_list +=glob.glob("auto_dsig*.f")
for file in file_list:
   print(file) 
   
