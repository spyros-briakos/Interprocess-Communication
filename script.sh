#Store array of arguments into an array.
args=("$@") 
#Check for exactly 3 arguments.
if [ "${#args[@]}" != "3" ]; then
    printf "You have to insert exactly these 3 arguments...\nNumber_Of_Workers BufferSize NameOfDirectory(ex. input_dir)\n"; exit 1
fi

make
./executable/diseaseAggregator -w $1 -b $2 -i $3
printf "\n"
make clean