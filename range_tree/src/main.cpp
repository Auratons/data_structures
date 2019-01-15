#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "bbalpha_tree.h"
#include "2Drange_tree.h"

using namespace rt;
using namespace std;

template<typename Out>
void split(const string &s, const char& delimiter, Out&& result) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delimiter)) {
        if (!item.empty())
            *(result++) = item;
    }
}

vector<string> split(const string &s, const char& delimiter) {
    vector<string> elems;
    split(s, delimiter, back_inserter(elems));
    return elems;
}

void print_stats(ofstream& ofs, const statistics_& stats) {
    ofs << stats.max_range_count_visits << " ";
    ofs << (stats.range_count_calls ? float(stats.range_count_visits) / stats.range_count_calls : 1) << " ";
    ofs << stats.max_insert_visits << " ";
    ofs << (stats.insert_calls ? float(stats.insert_visits) / stats.insert_calls : 1) << " ";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "One argument -- output file name." << endl;
        throw 1;
    }
    string line;
    size_t last_n = 0;
    volatile auto first = true;
    ofstream ofs{ argv[1] };

    statistics_ s1;
    statistics_ s2;
    statistics_ s3;

    range_tree_2d alpha1(0.52, s1);
    range_tree_2d alpha2(0.7, s2);
    range_tree_2d alpha3(0.98, s3);

    uint tree_count = 1;

    ofs << "#N a1_max_rng a1_mean_rng a1_max_ins a1_mean_ins " << \
              "a2_max_rng a2_mean_rng a2_max_ins a2_mean_ins " << \
              "a3_max_rng a3_mean_rng a3_max_ins a3_mean_ins" << endl;

    while (!getline(cin, line).eof()) {
        auto tokens = split(line, ' ');

        if (tokens[0] == string("#")) {
            if (!first) {
                ofs << last_n << " ";
                print_stats(ofs, s1);
                print_stats(ofs, s2);
                print_stats(ofs, s3);
                ofs << endl;
                cout << "TREE " << tree_count << " FINISHED!" << endl;
                ++tree_count;
            }
            // reset counters
            s1.reset();
            s2.reset();
            s3.reset();
            // prepare next run
            last_n = size_t(stoi(tokens[1]));
            alpha1.clear();
            alpha2.clear();
            alpha3.clear();
            first = false;
        }
        else if (tokens[0] == string("I")) {
            auto x1 = stoi(tokens[1]);
            auto y1 = stoi(tokens[2]);
            alpha1.insert(x1, y1);
            alpha2.insert(x1, y1);
            alpha3.insert(x1, y1);
        }
        else { // if (tokens[0] == string("C")) {
            auto x1 = stoi(tokens[1]);
            auto y1 = stoi(tokens[2]);
            auto x2 = stoi(tokens[3]);
            auto y2 = stoi(tokens[4]);
            alpha1.range_query(x1, y1, x2, y2);
            alpha2.range_query(x1, y1, x2, y2);
            alpha3.range_query(x1, y1, x2, y2);
        }
    }
    // Print last statistics.
    ofs << last_n << " ";
    print_stats(ofs, s1);
    print_stats(ofs, s2);
    print_stats(ofs, s3);
    ofs << endl;
    cout << "TREE " << tree_count << " FINISHED!" << endl;
    cout << "END OF INPUT" << endl;
}