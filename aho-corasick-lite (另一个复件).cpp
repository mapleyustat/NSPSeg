#ifndef  _AHO_CORASICK_LITE_
#define  _AHO_CORASICK_LITE_
// FastDictLoad.cpp : 定义控制台应用程序的入口点。
//

//#include <unordered_map>
#include <string.h>
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
	trie_node_s()
	{
		infoIndex = -1;
		nextMap.clear();
		search_clue = NULL;
	}

};


struct DAG_node
{
    int prev;
    uint32_t Weight;
    bool visited;
    // 优先队列 替代list直接取最大值
    list<uint8_t> nextPos;

    friend bool operator< (DAG_node n1, DAG_node n2) {
          return n1.Weight > n2.Weight;
    }

    DAG_node()
    {
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


public :
	Trie();
	bool insert(Unicode &_unicode);
	bool loadDict(const char *filePath);
	void traversalTrie(trie_node_t *node, int &total);
	void matchTextFile(const char *textFilePath);
	bool buildClue();
	uint32_t getSize() { return size; };
	bool findAllWithClue(const string &src, vector<string> &outVect);
	bool generateDAG(const string &src);
	trie_node_t *root;
	vector<uint32_t> weightVect;
	vector<uint8_t> lengthVect;
	int Dijkstra(DAG &_Dag);

};


inline uint32_t getPathWeight(const int &index,const int &step)
{
    auto iter = PathWeight.find({index, step});
    if (iter != PathWeight.end()) {
       return iter->second;
    }
    return 0;
}

int Trie::Dijkstra(DAG &_Dag)
{
    priority_queue<vector<DAG_node>::iterator> _Heap;
    auto i = _Dag.begin();
    int j;
    uint32_t curW, nextW;
    list<uint8_t>::iterator iter;
    _Heap.push(i);
    i->visited = true;
    do {
        i = _Heap.top();
        _Heap.pop();
        if (i->nextPos.empty() && ! (i+1)->visited ) {
           (i+1)->prev = (i-_Dag.begin());
           _Heap.push(i+1);
           (i+1)->visited = true;
           continue;
        }
        curW = i->Weight;
        for (iter = i->nextPos.begin(); iter != i->nextPos.end(); iter++) {
             j = *iter;
             nextW = curW + getPathWeight(int(i-_Dag.begin()), j);
             if (nextW < (i + j)->Weight) {
                 (i + j)->Weight = nextW;
                 (i + j)->prev = int(i - _Dag.begin());
             }
             if ( ! (i + j)->visited ) {
                _Heap.push(i + j);
                (i + j)->visited = true;
             }
        }

    } while ( ! _Heap.empty() );
}



bool Trie::generateDAG(const string &src)
{
	trie_node_t *node = root;
	uint16_t index;
    Unicode _unicode;
	decode(src, _unicode);
	DAG _dag(_unicode.size()+1);
	int head, tail, pos;
	auto iter = _unicode.begin();
	auto iterBegin = iter;
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
		tail = iter - iterBegin + 1;
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
	_dag[0].prev = -1;
	 int i, j;
    for (i = 1;i < _dag.size(); i++) _dag[i].prev = i -1;
    Dijkstra(_dag);
/*	for (i = 0;i < _dag.size(); i++) {
	    cout << _dag[i].prev << " ";
	    if (_dag[i].prev == -1) { cout << endl << " !!!! bug point !!!!" << endl;}
	}
*/	cout << endl;
	j = _dag.size()-1;
	stack<string> out;
	while (j > 0) {
	    // cout << j << " ";
	     i = _dag[j].prev;
	     encode(iterBegin+i, iterBegin+j, temp);
	     j = i;
	     out.push(temp);
	}
	cout << endl << endl << endl << out.size() << endl;
	while (!out.empty()) {
         cout << out.top() << " | " ;//<< out.size() << endl;
         out.pop();
     }
    cout << endl << endl;
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
	trie.traversalTrie(trie.root, total);
	time_e = clock();
	cout << " success in traversal in " << (double)(time_e - time_s)/CLOCKS_PER_SEC <<  " total Null nodes : " << total << endl;
	cout << " root node : " << (int)trie.root << endl;

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
	auto end = _unicode.end();
	for ( ; iter != end; iter++ ) {
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


bool Trie::findAllWithClue(const string &src, vector<string> &outVect)
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
}

void Trie::traversalTrie(trie_node_t *node, int &total)
 {
	if (node == NULL) {
		return;
	}
	if (node->search_clue == NULL) {
		printf("Null clue %d\n", (int)node);              total++;
	}
	if (node->nextMap.empty()) {
		return;
	}
	auto i = node->nextMap.begin();
	for ( ; i != node->nextMap.end(); i++) {
		traversalTrie(i->second, total);
	}
}

void Trie::matchTextFile(const char *textFilePath)
{
     ifstream ifs(textFilePath);
     if ( ! ifs ) {
         return;
     }
     string line, text;
     vector<string> outVect;
     while (getline(ifs, line, '\n')) {
           text += line;
     }
    cout << text << endl;
    generateDAG(text);
    cout << endl << endl;
 /*   findAllWithClue(text, outVect);
     auto iter = outVect.begin();
     for (; iter != outVect.end(); iter++) {
         cout << (*iter) << " | ";
     }
     cout << endl;
*/
}


#endif
