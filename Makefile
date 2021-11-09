.DEFAULT_GOAL:=full

clean:
	rm -rf build
full:
	mkdir build
	cd build && meson setup --buildtype release ..
	cd build && ninja
	cd build && ./spin_bench --benchmark_out=../result.json
	python3 viz.py
