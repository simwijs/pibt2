# $1 = K, $2 = dir_to_measure, $3 = task_prefix, $4 = output file, $5 = map_file $6 = task_file
# e.g. K=10, dir_to_measure=rq3_test, task_prefix=tasks/generatedb1_2
echo Settings: K=$1 dir_to_measure=$2 task_prefix=$3 output_file=$4 map_file=$5 >> $4
./build/mapd -m $5 -t $6 -s PIBT -o result.txt -b -p -c 1 -k $1 >> $4
mv output/task_* output/output* ../instances/output/pibt/$2
cd ../instances && python3 measurement.py $2 $3 && cd ../pibt2
./build/mapd -m $5 -t $6 -s PIBT -o result.txt -b -p -c 2 -k $1 >> $4
mv output/task_* output/output* ../instances/output/pibt/$2
cd ../instances && python3 measurement.py $2 $3 && cd ../pibt2
./build/mapd -m $5 -t $6 -s PIBT -o result.txt -b -p -c 3 -k $1 >> $4
mv output/task_* output/output* ../instances/output/pibt/$2
cd ../instances && python3 measurement.py $2 $3 && cd ../pibt2
./build/mapd -m $5 -t $6 -s PIBT -o result.txt -b -p -c 4 -k $1 >> $4
mv output/task_* output/output* ../instances/output/pibt/$2
cd ../instances && python3 measurement.py $2 $3 && cd ../pibt2

# RUN EXAMPLE
# Batchsize 1
# ./test_rq3_formulas.sh 1 rq3_test tasks/generatedb1_2 test_rq3.txt ../instances/maps/NS-20-500-5.map ../instances/tasks/generatedb1_2/0/NS-20-500-5.map-1-1-11-37-frame.task
# Batchsize 10
#./test_rq3_formulas.sh 1 rq3_test_batch10 tasks/generated test_rq3_batch10.txt ../instances/maps/NS-20-500-5.map ../instances/tasks/generated/0/NS-20-500-5.map-10-5-11-37-frame.task