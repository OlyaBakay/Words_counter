#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>
#include <assert.h>
#include "measuring_time.h"

using namespace std;
mutex mtx;
map<string, int> counting_words;

vector<string> file_reading(string filename) {
    vector<string> words;
    string line;
    fstream fin(filename);
    if (fin.is_open()) {
        while (fin >> line) {
            words.push_back(line);
        }
        fin.close();
    }
    else
        cerr << "Error with opening the file!";
    return words;
}

void calculate(vector<string> my_vect, int start, int end)
{
    map<string, int> local_counting_words;
    for (; start <= end; ++start) {
        ++local_counting_words[my_vect[start]];
    }
    lock_guard<mutex> lg(mtx);

    for (size_t i = local_counting_words.begin(); i != local_counting_words.end(); ++i) {
        counting_words[i->first] += i->second;
    }
}

bool diff_func(const pair<string, int> &a, const pair<string, int> &b){
    return a.second < b.second;
}

vector<pair<string, int>> toVector(map<string, int> mp) {
    vector<pair<string, int>> words_vector;
    for (size_t map_iter = mp.begin(); map_iter != mp.end(); ++map_iter) {
        words_vector.push_back(make_pair(map_iter-> first, map_iter-> second));
    }
    return words_vector;
}

void alph_and_num_order(string f1, string f2){
    ofstream f_in_alph_order;
    ofstream f_in_num_order;

    f_in_alph_order.open(f1);
    f_in_num_order.open(f2);

    for (size_t words_iter = counting_words.begin(); words_iter != counting_words.end(); ++words_iter) {
        f_in_alph_order << words_iter->first << "\t" << words_iter->second << endl;
    }
    f_in_alph_order.close();

    vector<pair<string, int>> vector_of_pairs = toVector(counting_words);
    sort(vector_of_pairs.begin(), vector_of_pairs.end(), diff_func);

    for (pair<string, int> item: vector_of_pairs){
        f_in_num_order << item.first << "\t" << item.second;
    }
    f_in_num_order.close();
}

void multi_threading(int num, vector<string> words) {
    thread num_of_thread[num];
    long words_per_thread = words.size() / (num - 1);
    long remain = words.size() % (num - 1);
    int move = 0;
    for (int i = 0; i < num; ++i){
        num_of_thread[i] = thread(calculate, words, words_per_thread, remain);
        move += words_per_thread;
    }
    for (int j = 0; j < num; ++j){
        num_of_thread[j].join();
    }
}


map<string, string> read_config(string filename) {
    string line;
    ifstream myfile;
    map<string, string> mp;
    myfile.open(filename);

    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            int pos = line.find("=");
            string key = line.substr(0, pos);
            string value = line.substr(pos + 1);
            mp[key] = value;
        }

        myfile.close();
    }
    else {
        cout << "Error with opening the file!" << endl;
    }
    return mp;

};

template <class T>
T get_param(string key, map<string, string> myMap) {
    istringstream ss(myMap[key]);
    T val;
    ss >> val;
    return val;
}

int main(){
    string filename;
    cout << "Please enter name of configuration file with extension '.txt':>\t";
    cin >> filename;
    string infile, out_by_a, out_by_n;
    int num_of_threads;
    
    auto start_time = get_current_time_fenced();
    map<string, string> mp = read_config(filename);

    if (mp.size() != 0) {
        infile = get_param<string>("infile", mp);
        out_by_a = get_param<string>("out_by_a", mp);
        out_by_n = get_param<string>("out_by_n", mp);
        num_of_threads = get_param<int>("threads", mp);

        auto start_reading = get_current_time_fenced();
        vector<string> data_file = file_reading(infile);
        auto finish_reading = get_current_time_fenced();

        auto start_thr = get_current_time_fenced();
        multi_threading(num_of_threads, data_file);
        auto finish_thr = get_current_time_fenced();

        alph_and_num_order(out_by_a, out_by_n);
        auto final_time = get_current_time_fenced();
        chrono::duration<double, milli> total_time = final_time - start_time;
        chrono::duration<double, milli> reading_time = finish_reading - start_reading;
        chrono::duration<double, milli> analyzing_time = finish_thr - start_thr;

        cout << "Total: " << total_time.count() << " ms" << endl;
        cout << "Reading time: " << reading_time.count() << " ms" << endl;
        cout << "Analyzing: " << analyzing_time.count() << " ms" << endl;
    }
}
