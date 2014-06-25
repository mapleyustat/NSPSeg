
#include "Trie.hpp"
#include "TrainMan.hpp"
#include <iostream>

using namespace std;

struct TagNode
{
    int prev;
    double prob;
    TagNode() : prev(0), prob(0) { }
};

// 词性概率 计算方式 abs(log10(w)) 概率P(w)不可能为1，故log10(w)必为负数，
// 故可将求最大转化为求负数数值的最小值。(c++ 不支持高精度计算，易发生精度溢出)

bool viterbi(const vector<string> &AllTags,
             const vector<string> &outString,
             const TrainMan &train,
             vector<string> &outTags)
{
   const int size = AllTags.size();
   TagNode **nodes; //[outString.size()][size];
// initialize base cases
   int i, cur, last, pos = outString.size() - 1;
   cout << outString.size() << endl << AllTags.size() << endl;
   nodes = new TagNode*[outString.size()];
   for (i=0; i < outString.size(); i++) {
	   nodes[i] = new TagNode[size];
   }
   string tag;
   for (i=0; i < size; i++) {
       tag = AllTags[i];
       nodes[0][i].prev = -1;
       nodes[0][i].prob = train.getStartProb(tag) +
                          train.getEmissionProb(outString[pos], tag);
   }
// run viterbi for i > 0
   double  maxProb, Prob;
   int index;
   for (i=1; i < outString.size(); i++) {
       for (cur=0; cur < size; cur++) {
		    maxProb = DBL_MAX;
            for (last=0; last < size; last++) {
                Prob = nodes[i-1][last].prob +
                             train.getTransProb(AllTags[last], AllTags[cur]) +
                             train.getEmissionProb(outString[pos-i], AllTags[cur]);
			    if ( Prob < maxProb ) {
					maxProb = Prob;
					index = last;
				}
            }
            nodes[i][cur].prev = index;
			nodes[i][cur].prob = maxProb;
       }
   }
   for (i=pos; i >= 0 ; i--) {
	   outTags.push_back(AllTags[index]);
	   index = nodes[i][index].prev;
   }
   for (i=0; i < outString.size(); i++) {
	   delete []nodes[i];
   }
   delete []nodes;
   return true;

}

bool exportPOSFile(const vector<string> &outString,
				   const vector<string> &outTags,
				   const char *filePath)
{
	if (filePath == nullptr) return false;
	ofstream posfs(filePath);
	if ( !posfs ) return false;
	posfs << outString.size() << '#' << outTags.size() << endl;
	int Tx =  outString.size() - 1;
	for (int index = Tx - 1; index; index--) {
		posfs << outString[index] << " /" << outTags[index] << ' ' ;
         if (!(index%20)) posfs << endl;
     }
	return true;
}

void segText(const char *dictPath, const char *destPath ,vector<string> &outString)
{

    time_t time_s, time_e;
	ShortPathSegment trie;
	time_s = clock();
	trie.loadDict(dictPath);
	time_e = clock();
	cout << " success in loadDict in " << (double)(time_e - time_s)/CLOCKS_PER_SEC << endl;
	cout << " insert " << trie.getSize() << " words" << endl;

	time_s = clock();
    trie.matchTextFile(destPath, outString);
    time_e = clock();
    cout << " success in matchFile in " << (double)(time_e - time_s)/CLOCKS_PER_SEC <<  endl;
    cout << "enter to continue\n";
    getchar();

}

void tagText(const char *modelPath,  const vector<string> &outString)
{
	time_t time_s, time_e;
    vector<string> outTags;

    TrainMan train;
    cout << "begin trianning file\n";
  	time_s = clock();
    train.trainFile(modelPath);
    time_e = clock();
    cout << " success in train in " << (double)(time_e - time_s)/CLOCKS_PER_SEC <<  endl;
    vector<string> AllTags;
    train.exportAllTags(AllTags);
    cout << "begin viterbi\n";
    time_s = clock();
    viterbi(AllTags, outString, train, outTags);
    time_e = clock();
    cout << " run viterbi in " << (double)(time_e - time_s)/CLOCKS_PER_SEC <<  endl;
    time_s = clock();
    exportPOSFile(outString, outTags, "out.txt");
    time_e = clock();
    cout << " success in exportPOSFile in " << (double)(time_e - time_s)/CLOCKS_PER_SEC <<  endl;
}

int main(int argc, char* argv[])
{

   if (argc < 4) {
	cout << "usage " << argv[0] << " dictfilePath trainfilePath destfilePath" << endl;
	return 0;
    }
    vector<string> outString;
    segText(argv[1], argv[3], outString);
    //tagText(argv[2], outString);
    cout << "enter to exit\n";
    getchar();

    return 0;
}
