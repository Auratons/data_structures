#ifndef TESTER_HPP
#define TESTER_HPP

#include <fstream>
#include <string>

template<typename T>
class tree_tester {
protected:
	std::ifstream ifile;
	std::ofstream ofile;

public:
	tree_tester() = default;
	tree_tester(const tree_tester&) = default;
	tree_tester(tree_tester&&) = default;
	tree_tester& operator=(const tree_tester&) = default;
	tree_tester& operator=(tree_tester&&) = default;

	void process(const std::string& i_file, const std::string& o_file) {
		ifile.open(i_file);
		ofile.open(o_file);
		if (!ifile.is_open() || !ofile.is_open()) {
			ifile.close();
			ofile.close();
			return;
		}
			
		std::string line;

		while (std::getline(ifile, line)) {
			process_line(line);
		}

		postprocess_last_line();

		ifile.close();
		ofile.close();
	}

	virtual ~tree_tester() = default;

protected:
	virtual void process_line(const std::string& line) = 0;
	virtual void postprocess_last_line() = 0;
};

#endif // TESTER_HPP