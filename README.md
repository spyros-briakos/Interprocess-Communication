# Interprocess-Communication
In this project I experiment with interprocess communication through named pipes and general low-level I/O.

# Summary
In this project basically the main goal of program is to answer to user's queries. We accomplish through this process: We have a main process,called diseaseAggregator, and some diseaseAggregator's children processes, called Workers. So each Worker manages with one or more countries and its responsibility is to give answers in diseaseAggregator's queries. From the other side diseaseAggregator collects Worker's answers (you can check out kinds of queries just right down in 'Runtime Options') and answer to user's query. 

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
