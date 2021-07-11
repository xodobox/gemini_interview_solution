.PHONY: build
build: ## Kicks off a build
	mkdir -p build
	/usr/bin/g++ -std=c++17 -o build/match_engine main.cpp
#/usr/bin/g++ -std=c++17 -o build/match_engine_test test.cpp catch_amalgamated.cpp

.PHONY: clean
clean: ## Cleans the build directory
	rm -rf build/*

