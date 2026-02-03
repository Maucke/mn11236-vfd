set "year=%date:~2,2%"
set "month=%date:~5,2%"
set "day=%date:~8,2%"
set "hour_ten=%time:~0,1%"
set "hour_one=%time:~1,1%"
set "minute=%time:~3,2%"
set "second=%time:~6,2%"
mkdir output
echo appheader v2.01 t%year%%month%%day%%hour_ten%%hour_one%%minute%%second% > "mn11236-vfd\appheader.bin"
copy /b mn11236-vfd\appheader.bin + mn11236-vfd\mn11236-vfd.bin output\mn11236-vfd_merge_20%year%%month%%day%.bin
del "mn11236-vfd\appheader.bin"

echo skip 32bytes