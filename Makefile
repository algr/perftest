PERF_EXE=perf
PERF_CMD=$(PERF_EXE) record -e intel_pt//u
WORK_EXE=workload
WORK_CMD=./$(WORK_EXE) 1 1 1

.PHONY: run
run: $(WORK_EXE)
	echo perftrace: default
	$(PERF_CMD) -o default.perf.data -vv -- $(WORK_CMD)
	echo perftrace: --per-thread
	$(PERF_CMD) -o perthread.perf.data -vv --per-thread -- $(WORK_CMD)
	echo perftrace: -a
	sudo $(PERF_CMD) -o all.perf.data -vv -a -- $(WORK_CMD)
	sudo chown $(USER) all.perf.data
	
workload: workload.cpp
	$(CXX) $< -o $@ -lpthread -static

.PHONY: clean
clean:
	rm -rf workload