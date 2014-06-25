#ifndef  _AHO_CORASICK_LITE_
#define  _AHO_CORASICK_LITE_

//#include <unordered_map>
#include <map>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <queue>
#include <list>
#include <stack>
#include <time.h>

#include "TransCode.hpp"

using namespace std;
using namespace CppJieba;
using namespace TransCode;

typedef struct trie_node_s trie_node_t;

#define MAX_UINT32 0xffffffff

map< pair<int, int> , uint32_t > PathWeight;

typedef map<uint16_t, trie_node_t*> NodeMap;

struct trie_node_s
{
	int infoIndex;
	NodeMap nextMap;
	trie_node_t *search_clue;

	trie_node_s() {
		infoIndex = -1;
		search_clue = NULL;
	}

};


struct DAG_node
{
    int prev;
    uint32_t Weight;
    bool visited;
    list<uint8_t> nextPos;

    friend bool operator< (DAG_node n1, DAG_node n2) {
          return n1.Weight > n2.Weight;
    }

    DAG_node() {
		prev = -1;
		Weight = MAX_UINT32;
		visited = false;
    }
};

typedef vector<DAG_node> DAG;

class Trie
{

private :
	uint32_t size;
    vector<uint32_t> weightVect;
	vector<uint8_t> lengthVect;
	trie_node_t *root;


public :
	Trie();
	bool insert(Unicode &_unicode);
	bool loadDict(const char *filePath);
	void matchTextFile(const char *textFilePath);
	bool buildClue();
	uint32_t getSize() const { return size; };
	bool matchAll(const string &src, vector<string> &outVect);
	bool generateDAG(Unicode &_unicode, DAG &_dag);

public:
	friend bool Dijkstra(DAG &_Dag);

};


inline uint32_t getPathWeight(const int &index,const int &step)
{
    auto iter = PathWeight.find({index, step});
    if (iter != PathWeight.end()) {
       return iter->second;
    }
    return 0;
}

bool Dijkstra(DAG &_Dag)
{
    priority_queue<vector<DAG_node>::iterator> _minHeap;
    auto loc = _Dag.begin();
    auto next = loc;
    int j;
    uint32_t curW, nextW;
    list<uint8_t>::iterator iter;
    _minHeap.push(loc);
    loc->visited = true;
    do {
        loc = _minHeap.top();
        _minHeap.pop();
        if (loc >= _Dag.end()) continue;
        if (loc->nextPos.empty()) {
           next = loc + 1;
           if (! next->visited) {
             _minHeap.push(next);
             next->visited = true;
           }
           continue;
        }
        curW = loc->Weight;
        for (iter = loc->nextPos.begin(); iter != loc->nextPos.end(); iter++) {
             j = *iter;
             nextW = curW + getPathWeight(int(loc-_Dag.begin()), j);
             next = loc + j;
             if (nextW < next->Weight) {
                 next->Weight = nextW;
                 next->prev = int(loc - _Dag.begin());
             }
             if ( ! next->visited ) {
                _minHeap.push(next);
                next->visited = true;
             }
        }

    } while ( ! _minHeap.empty() );
    return true;
}



bool Trie::generateDAG(Unicode &_unicode, DAG &_dag)
{
	trie_node_t *node = root;
	uint16_t index;
	int head, tail, pos;
	auto iter = _unicode.begin();
	auto begin = iter;
    for( ; iter != _unicode.end(); iter++ ) {
		index = *iter;
		for ( ; ; ) {
			if ( node == NULL ) {
				break;
			} else if ( node->nextMap.find(index) == node->nextMap.end() ) {
			    node = node->search_clue;
			} else {
				node = node->nextMap[index];
				break;
			}
		}
		tail = iter - begin + 1;
	    if ( node == NULL ) {
			node = root;
			_dag[tail - 1].nextPos.push_back(1);
		} else	{
			if ( node->infoIndex != -1 ) {
			    pos = node->infoIndex;
                head = tail - lengthVect[pos];
			    PathWeight.insert({{head, lengthVect[pos]}, weightVect[pos]});
			    _dag[head].nextPos.push_back(lengthVect[pos]);
			}
		}
	}
	_dag[0].Weight = 0;
    for (int i = 0;i < _dag.size(); i++) {
        _dag[i].prev = i -1;
    }

	return true;
}

void printOutString(stack<string> &out)
{
	cout << endl << endl;
	while (!out.empty()) {
         cout << out.top() << " | " ;
         out.pop();
     }
    cout << endl << endl;
}

bool decodeOutString(DAG &_dag, stack<string> &out, Unicode _unicode)
{
	int i, j = _dag.size()-1;
	auto begin = _unicode.begin();
	string temp;
	while (j > 0) {
	     i = _dag[j].prev;
	     encode(begin+i, begin+j, temp);
	     j = i;
	     out.push(temp);
	}
    return true;
}

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
    trie.matchTextFile(argv[2]);
    time_e = clock();
    cout << " success in matchFile in " << (double)(time_e - time_s)/CLOCKS_PER_SEC <<  " total Null nodes : " << endl;

    getchar();
	return 0;
}

Trie::Trie()
{
	root = new trie_node_t;
	size = 0;
}


bool Trie::loadDict(const char *filePath)
{
	ifstream ifs(filePath);
	if ( ! ifs ) {
		cout << "faild to load Dict file" << endl;
		return false;
	}
	string line;
	uint32_t w;
    Unicode _unicode;
	while ( !ifs.eof()) {
		ifs >> line >> w;
	    decode(line, _unicode);
        weightVect.push_back(w);
		lengthVect.push_back(_unicode.size());
		insert(_unicode);
		ifs.ignore(64, '\n');
	}
	ifs.close();
	return true;
}

bool Trie::insert(Unicode &_unicode)
{
	trie_node_t *node = root;
	uint16_t index;
	auto iter = _unicode.begin();
	for ( ; iter != _unicode.end(); iter++ ) {
		index = *iter;
		if ( node->nextMap.find(index) == node->nextMap.end() ) {
			try {
				node->nextMap[index] = new trie_node_t;
			}
		    catch(const bad_alloc& ) {
				return false;
			}
		}
		node = node->nextMap[index];
	}
	node->infoIndex = size++;
	return true;
}


bool Trie::matchAll(const string &src, vector<string> &outVect)
{
	trie_node_t *node = root;
	uint16_t index;
    Unicode _unicode;
	decode(src, _unicode);
	auto iter = _unicode.begin();
	auto begin = iter, last = iter;
	auto end = _unicode.end();
	string temp;
    for( ; iter != end; iter++ ) {
		index = *iter;
		for ( ; ; ) {
			if ( node == NULL ) {
				break;
			} else if ( node->nextMap.find(index) == node->nextMap.end() ) {
			    node = node->search_clue;
			} else {
				node = node->nextMap[index];
				break;
			}
		}
	    if ( node == NULL ) {
			node = root;
		} else	{
			if ( node->infoIndex != -1 ) {
				temp.clear();
				last = iter - lengthVect[node->infoIndex] + 1;
                encode(last, iter+1 , temp);
                outVect.push_back(temp);
			}
		}
	}
	return true;
}

bool Trie::buildClue()
{
	queue<trie_node_t*> Queue;
	auto iter = root->nextMap.begin();
	int i, head = 0, tail = 0;
	trie_node_t *clue = root, *out, *child;
	Queue.push(root);
	while ( ! Queue.empty() ) {
		out = Queue.front();
		Queue.pop();
		if (out->nextMap.empty()) {
			continue;
		}
		for (iter = out->nextMap.begin(); iter != out->nextMap.end(); iter++) {
			child = iter->second;
			if (child != NULL) {
				Queue.push(child);
				if (out == root) {
					child->search_clue = root;
					continue;
				}
				clue = out->search_clue;
				i = iter->first;
				while (clue) {
					if (clue->nextMap.find(i) != clue->nextMap.end()) {
						child->search_clue = clue->nextMap[i];
						break;
					}
					clue = clue->search_clue;
				}
				if (clue == NULL) {
					child->search_clue = root;
				}
			}
		}
	}
	return true;
}

void Trie::matchTextFile(const char *textFilePath)
{
     ifstream ifs(textFilePath);
     if ( ! ifs ) {
         return;
     }
     string line, text;
     while (getline(ifs, line, '\n')) {
           text += line;
     }
    cout << text << endl;
    Unicode _unicode;
    stack<string> outString;
    decode(text, _unicode);
    cout << " decode end\n";
	DAG _dag(_unicode.size()+1);
    generateDAG(_unicode, _dag);
    cout << "generateDAG end\n";
    Dijkstra(_dag);
    cout << "dijkstra end\n";
    decodeOutString(_dag, outString , _unicode);
    printOutString(outString);
}


#endif
