
rm -rf profile.csv

max=100
for i in `seq 1 $max`
do
    ./profile.exe a 2>> profile.csv
done

echo "" >> profile.csv

for i in `seq 1 $max`
do
    ./profile.exe b 2>> profile.csv
done

echo "" >> profile.csv

for i in `seq 1 $max`
do
    ./profile.exe c 2>> profile.csv
done

echo "" >> profile.csv

for i in `seq 1 $max`
do
    ./profile.exe d 2>> profile.csv
done

echo "" >> profile.csv
