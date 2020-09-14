# Interprocess-Communication-
In this project I experiment with interprocess communication through named pipes and general low-level I/O.

# Summary
In this project 

# Execution 
You can check it out with this command and this exact order: 

**bash script.sh Number_Of_Workers BufferSize NameOfDirectory(ex. input_dir)** 

# Runtime options
1) /listCountries
2) /diseaseFrequency virusName date1 date2 (country)
3) /topk-AgeRanges k country disease date1 date2
4) /searchPatientRecord recordID
5) /numPatientAdmissions disease date1 date2 (country)
6) /numPatientDischarges disease date1 date2 (country)
7) /exit

Note: (...) means that this parameter is optional!

# create_infiles.sh 
This is just a script that creates a directory which is gonna be the input of our main program (3rd parameter). 
If you wanna make one directory you should run something like this:

**./create_infiles.sh File_with_diseases File_with_countries Name_of_dir numFilesPerDirectory numRecordsPerFile**
