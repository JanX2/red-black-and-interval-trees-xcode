//
//  main.cpp
//  interval_tree_test3
//
//  Created by Jan on 13.11.12.
//  Copyright (c) 2012 Jan. All rights reserved.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <time.h>
#include <assert.h>

#include "interval_tree.h"
#include "TemplateStack.hpp"

using namespace std;

template<class T, typename K = int>
class IntervalTemplate : public Interval {
public:
	IntervalTemplate(const int low, const int high, const T& v)
    :_low(low), _high(high), _value(v)
    { }
	
	int GetLowPoint() const { return _low;}
	int GetHighPoint() const { return _high;}
	const T& GetValue() const { return _value;}
	//IntervalTreeNode * GetNode() { return _node;}
	//void SetNode(IntervalTreeNode * node) {_node = node;}
protected:
	int _low;
	int _high;
    T _value;
	//IntervalTreeNode * _node;
};

typedef IntervalTemplate<bool> interval;
typedef vector<interval> intervalVector;
typedef IntervalTree intervalTree;



template<typename K>
K randKey(K floor, K ceiling) {
    K range = ceiling - floor;
    return floor + range * ((double) rand() / (double) (RAND_MAX + 1.0));
}

template<class T, typename K>
IntervalTemplate<T,K> randomInterval(K maxStart, K maxLength, K maxStop, const T& value) {
    K start = randKey<K>(0, maxStart);
    K stop = min<K>(randKey<K>(start, start + maxLength), maxStop);
    return IntervalTemplate<T,K>(start, stop, value);
}

int main() {
    typedef vector<std::size_t> countsVector;
	
#if 0
	srand((unsigned)time(NULL));
#else
	// This will result in test data and a tree that is identical every time as long as the constant below remain unchanged.
	srand(0);
#endif
	
	int requestedIntervalsCount = 10000;
	int requestedQueriesCount = requestedIntervalsCount / 2;

	int intervalValueLimit = 100000;
	int intervalLengthLimit = 1000;

    intervalVector intervals;
    intervalVector queries;
	
    // generate a test set of target intervals
    for (int i = 0; i < requestedIntervalsCount; ++i) {
        intervals.push_back(randomInterval<bool>(intervalValueLimit, intervalLengthLimit, intervalValueLimit + 1, true));
    }
    // and queries
    for (int i = 0; i < requestedQueriesCount; ++i) {
        queries.push_back(randomInterval<bool>(intervalValueLimit, intervalLengthLimit, intervalValueLimit + 1, true));
    }
	
    typedef chrono::high_resolution_clock Clock;
    typedef chrono::milliseconds milliseconds;
	
    // using brute-force search
    countsVector bruteForceCounts;
    Clock::time_point t0 = Clock::now();
    for (intervalVector::iterator q = queries.begin(); q != queries.end(); ++q) {
        intervalVector results;
        for (intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
            if (i->GetLowPoint() >= q->GetLowPoint() && i->GetHighPoint() <= q->GetHighPoint()) {
                results.push_back(*i);
            }
        }
        bruteForceCounts.push_back(results.size());
    }
    Clock::time_point t1 = Clock::now();
    milliseconds ms = chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "brute force:\t" << ms.count() << "ms" << endl;
	
	// building the interval tree
    t0 = Clock::now();
	intervalTree tree = intervalTree(/*intervals*/);
	for (intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
		tree.Insert(&(*i));
	}
    t1 = Clock::now();
    ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "building interval tree:\t" << ms.count() << "ms" << endl;

    // using the interval tree
	countsVector treeCounts;
    t0 = Clock::now();
    for (intervalVector::iterator q = queries.begin(); q != queries.end(); ++q) {
        TemplateStack<void *> *results;
        results = tree.EnumerateContained(q->GetLowPoint(), q->GetHighPoint());
        treeCounts.push_back(results->Size());
		delete results;
    }
    t1 = Clock::now();
    ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "using interval tree:\t" << ms.count() << "ms" << endl;
	
    // check that the same number of results are returned
    countsVector::iterator b = bruteForceCounts.begin();
    for (countsVector::iterator t = treeCounts.begin(); t != treeCounts.end(); ++t, ++b) {
        assert(*b == *t);
    }
	
    return 0;
}

