#include <string>
#include <iostream>

#include "tester_splay_tree.h"
#include "tester_statical_opt_tree.h"

using namespace std;
using namespace tp;


int main(int argc, char * argv[]) {
	ios_base::sync_with_stdio(false);
	statical_tree_tester<size_t> statical;
	splay_tree_tester<size_t> std("std");
	splay_tree_tester<size_t> naive("naive");

	std.process("../subset10.txt", "sub10_std.data");
	std::cout << "end" << std::endl;
	/*
	// 1st 
	std.process("../subset10.txt", "sub10_std.data");
	std::cout << "end" << std::endl;
	std.process("../subset100.txt", "sub100_std.data");
	std::cout << "end" << std::endl;
	std.process("../subset1000.txt", "sub1000_std.data");
	std::cout << "end" << std::endl;
	std.process("../subset10000.txt", "sub10000_std.data");
	std::cout << "end" << std::endl;
	std.process("../subset100000.txt", "sub100000_std.data");
	std::cout << "end" << std::endl;
	std.process("../subset1000000.txt", "sub1000000_std.data");
	std::cout << "end" << std::endl;

	//2nd
	naive.process("../subset10.txt", "sub10_std.data");
	std::cout << "end" << std::endl;
	naive.process("../subset100.txt", "sub100_std.data");
	std::cout << "end" << std::endl;
	naive.process("../subset1000.txt", "sub1000_std.data");
	std::cout << "end" << std::endl;
	naive.process("../subset10000.txt", "sub10000_std.data");
	std::cout << "end" << std::endl;
	naive.process("../subset100000.txt", "sub100000_std.data");
	std::cout << "end" << std::endl;
	naive.process("../subset1000000.txt", "sub1000000_std.data");
	std::cout << "end" << std::endl;

	// 4th
	std.process("../uniform.txt", "uni_std.data");
	std::cout << "end" << std::endl;
	statical.process("../uniform.txt", "uni_sta.data");
	std::cout << "end" << std::endl;

	// 5th
	std.process("../nonuniform.txt", "non_std.data");
	std::cout << "end" << std::endl;
	statical.process("../nonuniform.txt", "non_sta.data");
	std::cout << "end" << std::endl;

	// 6th
	std.process("../sequential.txt", "seq_std.data");
	std::cout << "end" << std::endl;
	naive.process("../sequential.txt", "seq_naive.data");
	std::cout << "end" << std::endl;
	
	// 7th
	statical.process("../sequential.txt", "seq_sta.data");
	std::cout << "end" << std::endl;
	std::string x;
	std::getline(std::cin, x);*/
}