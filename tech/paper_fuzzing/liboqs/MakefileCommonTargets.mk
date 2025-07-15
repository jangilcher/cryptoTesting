RUN_ALL_BAR_POS?=0

run_all_python: all
	python3 run_all.py python --base_path aggr_python --bar_pos ${RUN_ALL_BAR_POS}

run_all_afl: all
	python3 run_all.py afl --base_path aggr_afl --bar_pos ${RUN_ALL_BAR_POS}