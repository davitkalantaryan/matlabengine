#!/bin/bash


lsbCodeName=`lsb_release -c | cut -f 2`
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
curDir=`pwd`

export MATLABPATH=$scriptDir/../../sys/$lsbCodeName/mbin:$scriptDir/../src/matlabscripts:$MATLABPATH
export PATH=$scriptDir/../../sys/$lsbCodeName/bin:$PATH
export LD_LIBRARY_PATH=$scriptDirectory/../../sys/$lsbCodeName/dll:$scriptDir/../../sys/$lsbCodeName/lib:$LD_LIBRARY_PATH


cd $curDir
