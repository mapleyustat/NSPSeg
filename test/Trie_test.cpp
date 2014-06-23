
#include "Trie.hpp"
#include <iostream>

using namespace std;




int main(int argc, char* argv[])
{
	if (argc < 3) {
	    cout << "usage " << argv[0] << " dictfilePath textfilePath" << endl;
	    return 0;
	}

	Trie trie;
	time_t time_s, time_e;
	time_s = clock();
	trie.loadDict(argv[1]);
	time_e = clock();
	cout << " success in loadDict in " << (double)(time_e - time_s)/CLOCKS_PER_SEC << endl;
	cout << " insert " << trie.getSize() << " words" << endl;

	cout << "begin build clue" << endl;
	time_s = clock();
	trie.buildClue();
	time_e = clock();
	cout << " success in build clue in " << (double)(time_e - time_s)/CLOCKS_PER_SEC << endl;

	int total = 0;
	time_s = clock();
	//trie.traversalTrie(trie.root, total);
	time_e = clock();
	cout << " success in traversal in " << (double)(time_e - time_s)/CLOCKS_PER_SEC <<  " total Null nodes : " << total << endl;
	//cout << " root node : " << (int)trie.root << endl;

	time_s = clock();
    trie.matchTextFile(argv[2]);
    time_e = clock();
    cout << " success in matchFile in " << (double)(time_e - time_s)/CLOCKS_PER_SEC <<  " total Null nodes : " << endl;

    getchar();
	return 0;
}
