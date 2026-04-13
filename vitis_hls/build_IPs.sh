#!/bin/bash

vitis_test=`which vitis_hls | wc -l`
if [[ $vitis_test -ne 1 ]]; then
	echo No vitis_hls set!
	exit
fi

dir_list=(*/)
mkdir IPs

read -p "Export IPs?[y/n]" option
case $option in
	[Yy]* )
		for dir in "${dir_list[@]}"; do
			echo "Building IP for $dir"
			cd $dir
			vitis_hls build.tcl
			cd ../
		done
	;;
esac

read -p "Continue to extract exported files?[y/n]" option
case $option in
	[Nn]* ) exit;;
esac


export_list=($(find . -name export.zip))

for exports in "${export_list[@]}"; do
	echo "Exporting $exports"
	working_dir=($(echo $exports | tr '/' '\n'))
	echo ${working_dir[1]}
	unzip -q $exports -d IPs/${working_dir[1]}/
done
