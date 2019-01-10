#include "bloom_filter.h"

int main(int argc, char* argv[]) {
	bloom_filter<1000, 15> hash_set;
	char str[] = "aa a a a";
	auto x = hash_set.insert(str, 8 * sizeof(char));
}