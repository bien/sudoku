CXXFLAGS = -g -Wall -O2

solve: solve.o
	$(CXX) $^ -o $@

clean:
	rm -f solve solve.o
