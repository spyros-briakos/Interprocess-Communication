default: inactive_worker worker diseaseAggregator
diseaseAggregator: diseaseaggregator.o helpdiseaseAggregator.o
	g++ -std=c++11 build/diseaseaggregator.o build/helpdiseaseAggregator.o -o executable/diseaseAggregator
inactive_worker: inactive_worker.o
	g++ -std=c++11 build/inactive_worker.o -o executable/inactive_worker
worker: worker.o helpworker.o binary_tree.o record_list.o bucket_list.o max_heap.o
	g++ -std=c++11  build/binary_tree.o build/record_list.o build/bucket_list.o build/max_heap.o build/helpworker.o build/worker.o -o executable/worker 
diseaseaggregator.o:
	g++ -std=c++11 -c -o build/diseaseaggregator.o source/diseaseaggregator.cpp
helpdiseaseAggregator.o:
	g++ -std=c++11 -c -o build/helpdiseaseAggregator.o source/helpdiseaseAggregator.cpp
inactive_worker.o:
	g++ -std=c++11 -c -o build/inactive_worker.o source/inactive_worker.cpp
worker.o:
	g++ -std=c++11 -c -o build/worker.o source/worker.cpp
helpworker.o:
	g++ -std=c++11 -c -o build/helpworker.o source/helpworker.cpp
record_list.o: 
	g++ -std=c++11 -c -o build/record_list.o source/record_list.cpp
bucket_list.o: 
	g++ -std=c++11 -c -o build/bucket_list.o source/bucket_list.cpp
binary_tree.o: 
	g++ -std=c++11 -c -o build/binary_tree.o source/binary_tree.cpp
max_heap.o: 
	g++ -std=c++11 -c -o build/max_heap.o source/max_heap.cpp
clean:
	rm build/binary_tree.o build/bucket_list.o build/max_heap.o build/record_list.o build/helpworker.o build/worker.o build/diseaseaggregator.o build/helpdiseaseAggregator.o build/inactive_worker.o executable/diseaseAggregator executable/worker executable/inactive_worker pipes/PCfifo* pipes/CPfifo* log_files/log_file.*