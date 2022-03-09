#!/bin/bash
GREEN='\e[00;1;32m'
BLUE='\e[00;1;34m'
RED='\e[00;1;31m'
WHITE='\e[00;1;37m'
RESET='\e[m'

#do not edit
EXTC='.conf'
EXTO='.csv'

#default (overwrite if required)  important! leave no spaces in VAL strings i.e. DO VAL=( "1,2" "3,4" "5,6")  NOT VAL = ( "1 , 2" "3, 4" "5 ,6")
KEY='default_env'
VAL=( "0" )


#!!!This script allows multiple runs of the simulation with the option to overwrite a particular parameters
# of interest on each run. Begin with a core config file (FILENAME) and then you can overwrite ANY
# KEY parameter by setting its VALues.
# The resulting output file will be FILENAME_KEY.csv  !!!!

#It's also possible to log the bioburden on each surface throughout (warning = big files!)
# output filename similar as above but pre-appended with 'bio_'

#choose your configuration file here (without file extensions) !
#note: do not modify/save file during a simulation as this will alter the parameters in between loops
FILENAME='2z_risk'

#Number of realisations to average over
REPS='40000'

#Log bioburden (0 no, 1 yes)
LOGB='0'

#Select a KEY i.e. parameter to alter over multiple simulations and set their value (VAL) in each
#In this way you can build up simulation results as parameters change (and plot a nice graph!)
#e.g. current (uncommented) KEY/VAL will alter the parameters of the touch-screen (no. of touches require to operate it)



#KEY='PATHOGEN_INOC_PERIOD_MIN'
#VAL=( "1" "2" "5" "8" "10" "20" "30" "40" "60" "80" "90" "100" "200" "400" "800" "1000" "2000")


#KEY='PATHOGEN_ID50'
#VAL=( "100000" "50000" "10000" "5000" "1000" "500" "200" "100" "80" "40" "10" "5" "1" )
#KEY='PEOPLE'
#VAL=( "1,1,1" "5,1,1" "10,1,1" "20,1,1" "50,1,1" "100,1,1" )

#KEY='CLEAN_RATE_HR'
#VAL=( "0,0" "1,0" "2,0" "5,0" "10,0" "20,0" )
#VAL=( "0,0" "0,1" "0,2" "0,5" "0,10" "0,20" )
#VAL=( "0,0" "1,1" "2,2" "5,5" "10,10" "20,20" )


#KEY='FOMITES'
#VAL=( "1,1" "1,2" "1,3" "1,4" "1,5" "1,8" "1,10" "1,12")
##VAL=( "1,1" "2,1" "3,1" "4,1" "5,1" "8,1" "10,1" "12,1")

#KEY='PATHOGEN_DECAY_FOMITE'
#VAL=( "0.0000001" "0.0001" "0.001" "0.01" "0.05" "0.10" "0.20" "0.4" "0.8" "0.999" )


#KEY='PATHOGEN_SHED'
#VAL=( "10,10" "50,50" "100,100" "500,500" "1000,1000" "5000,5000" "10000,10000" "50000,50000" "100000,100000" )

KEY='mcd_menu'
VAL=( "0,0,1,40,0.5" "1,3.5,1,40,0.5" "4,3.5,1,40,0.5" "8,3.5,1,40,0.5" "16,3.5,1,40,0.5" "32,3.5,1,40,0.5" )

#KEY='ENVR'
#VAL=( "0,0,0,1" "0.01,0.1,0,0.6" "0.05,0.1,0,0.6" "0.10,0.1,0,0.6" "0.15,0.1,0,0.6" "0.20,0.1,0,0.6" "0.25,0.1,0,0.6" "0.30,0.1,0,0.6" "0.40,0.1,0,0.6" )

#Input configuration base file
CONFIG="$FILENAME$EXTC"
#Output csv file
OUTPUT="${FILENAME}_${KEY}$EXTO"


echo -e "${GREEN}running sim on KEY:${KEY} ${RESET}\n"

for i in "${VAL[@]}"
do
	./sim $CONFIG $OUTPUT $REPS $LOGB $KEY $i
	 echo -e "\n\n"
done


echo -e "${GREEN}Simulations complete${RESET}\n"
notify-send 'Sim Complete'
