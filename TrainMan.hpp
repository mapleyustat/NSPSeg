#ifndef  TRAINMAN_H_
#define  TRAINMAN_H_

#include <float.h> // for DBL_MIN
#include <fstream>
#include <vector>
#include <stdint.h>
#include <cmath>
#include <string>
#include <sstream>
#include <map>

using namespace std;

class TrainMan
{
private :
    map<string, double> tag_counter;
    map<string, double> start_prob;
    map<pair<string, string>, double> trans_prob;
    map<pair<string, string>, double> emission_prob;
    uint32_t start_tags;
	uint32_t total_tags;

private :
    bool calcAllProb();
    template <class ProbMap, class Key>
    inline void addTF(ProbMap &probMap, Key key);

public :
    bool exportAllTags(vector<string> &AllTags);
    bool trainFile(const char *filePath);
    bool exportTagFile(const char *filePath);
    bool exportTransProbFile(const char *filePath);
    bool exportEmissionProbFile(const char *filePath);
    bool explortStartProbFile(const char *filePath);

public :
    double getStartProb(const string &tag) const;
    double getTransProb(const string &last, const string &cur) const;
    double getEmissionProb(const string &word, const string &tag ) const;
	bool   resetTrainZero();
};

bool TrainMan::resetTrainZero()
{
	start_tags = 0;
	total_tags = 0;
	tag_counter.clear();
	start_prob.clear();
	trans_prob.clear();
	emission_prob.clear();
	
	return true;
}

double TrainMan::getTransProb(const string &last, const string &cur) const
{
    auto iter = trans_prob.find(make_pair(last, cur));
    if (iter != trans_prob.end()) {
        return iter->second;
    }
	return abs(log10(1.0/total_tags));
}

double TrainMan::getStartProb(const string &tag) const
{
    auto iter = start_prob.find(tag);
    if (iter != start_prob.end()) {
        return iter->second;
    }
	return abs(log10(1.0/start_tags));
}

double TrainMan::getEmissionProb(const string &word, const string &tag) const
{
    auto iter = emission_prob.find(make_pair(word, tag));
    if (iter != emission_prob.end()) {
        return iter->second;
    }
	return abs(log10(1.0/total_tags));
}

bool TrainMan::exportAllTags(vector<string> &AllTags)
{
    for (auto i : tag_counter) {
         AllTags.push_back(i.first);
    }
    return true;
}

template <class ProbMap, class Key>
inline void TrainMan::addTF(ProbMap &probMap, Key key)
{
     auto iter = probMap.find(key);
     if (iter == probMap.end())
         probMap.insert( make_pair(key, 1) );
     else
         iter->second++;
}


bool TrainMan::calcAllProb()
{
    for (auto &i : start_prob) {
		i.second = abs(log10(i.second/start_tags));
    }
    for (auto &i : emission_prob) {
		i.second = abs(log10(i.second/tag_counter[i.first.second]));
    }
    for (auto &i : trans_prob) {
		i.second = abs(log10(i.second/tag_counter[i.first.first]));
    }
    return true;
}

bool TrainMan::exportTagFile(const char *filePath)
{
    if (filePath == nullptr) return false;
    ofstream tag_fs(filePath);
    if ( !tag_fs )  return false;
    for (auto i : tag_counter) {
        tag_fs << i.first << "\t " << i.second << endl;
    }
    tag_fs.close();
    return true;
}

bool TrainMan::exportTransProbFile(const char *filePath)
{
    if (filePath == nullptr) return false;
    ofstream trans_prob_fs(filePath);
    if (! trans_prob_fs) return false;
    for (auto &i : trans_prob) {
        trans_prob_fs << i.first.first <<  "\t"
        << i.first.second << "   \t" << i.second << endl;
    }
    trans_prob_fs.close();
    return true;
}

bool TrainMan::exportEmissionProbFile(const char *filePath)
{
    ofstream emission_prob_fs(filePath);
    if (! emission_prob_fs ) return false;
    for (auto &i : emission_prob) {
        emission_prob_fs << i.first.first <<  "      \t"
        << i.first.second << "   \t" << i.second << endl;
    }
    emission_prob_fs.close();
    return true;
}

bool TrainMan::explortStartProbFile(const char *filePath)
{
    ofstream start_prob_fs(filePath);
    if (! start_prob_fs) return false;
    for (auto &i : start_prob) {
        start_prob_fs << i.first << "\t " << i.second << endl;
    }
    start_prob_fs.close();
    return true;
}


bool TrainMan::trainFile(const char *filePath)
{
    ifstream ifs(filePath);
    if (! ifs)   return false;
    string lines, word, substr, state, prevState;
    vector<string> state_buf;
    int pos, last;
	start_tags = 0;
	total_tags = 0;
    while (getline(ifs, lines)) {
        prevState.clear();
        // start_prob
        pos = lines.find('/');
        if (pos == string::npos) {
           continue;
        }
        start_tags++;
        last = lines.find(' ', pos);
        state = lines.substr(pos+1, last-pos-1);
        addTF(start_prob, state);
        prevState = state;
        // trans_prob
        stringstream linestream(lines);
        while (linestream >> word) {
            pos = word.find('[');
            // nt nz..
            if (string::npos != pos) {
                state_buf.clear();
                last = word.rfind('/');
                substr = word.substr(pos+1, last-pos-1);
                state = word.substr(last+1);
                state_buf.push_back(state);
                linestream >> word;
                while ((pos = word.rfind(']')) == string::npos) {
                    last = word.rfind('/');
                    substr += word.substr(0, last);
                    state = word.substr(last+1);
                    state_buf.push_back(state);
                    linestream >> word;
                }
                last = word.rfind('/');
                substr += word.substr(0, last);
                state = word.substr(last+1, pos-last-1);
                state_buf.push_back(state);
                state = word.substr(pos+1);
                state_buf.push_back(state);
                for (auto i : state_buf) {
                    addTF(tag_counter, i);
                    addTF(trans_prob, make_pair(prevState, i));
                    addTF(emission_prob, make_pair(substr, i));
                    prevState = i;
					total_tags++;
                }
                continue;
            }
            // other
            pos = word.rfind('/');
            substr = word.substr(0, pos);
            state = word.substr(pos+1, word.length());
            addTF(tag_counter, state);
            addTF(trans_prob, make_pair(prevState, state));
            addTF(emission_prob, make_pair(substr, state));
			total_tags++;
        }
    }
    ifs.close();
	total_tags += start_tags;
    calcAllProb();
    return true;
}

#endif
