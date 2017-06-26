#include <iostream>
#include <list>
#include <vector>
#include <cassert>

#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <queue>
#include <set>
#include <map>
#include <algorithm>
using namespace std;

namespace {

#define DEBUG(a1)    cout << a1 << endl;
#define DEBUG2(a1, a2)    cout << a1 << "\t" << a2 << endl;
#define DEBUG_NM(x)    cout << ""#x" = " << x << endl;

#define ASRT_FAULT(msg)    \
    do{bool msg = false;\
    assert (msg);}while(0)

    struct sArgs{
        enum eParseArgsState{
            SEARCH_N,
            READ_N,
            READ_I,
            READ_O,
            END,
        };

        size_t N;
        string inputFileName;
        string outputFileName;
        sArgs()
        {
            N = 10;
            inputFileName = "";
            outputFileName = "";
        }

        void parseArgs(char *argv[], int argc)
        {
            eParseArgsState state = SEARCH_N;
            for (int i = 1; i < argc; ++i)
            {
                switch (state) {
                case SEARCH_N:
                {
                    if (argv[i] == string("-n"))
                    {
                        state = READ_N;
                    }
                    else
                    {
                        --i;
                        state = READ_I;
                    }
                    break;
                }
                case READ_N:
                {
                    string potentialNString(argv[i]);

                    assert (potentialNString.size() <= 3);

                    stringstream convert(potentialNString);

                    int potentialN = -1;
                    assert (convert >> potentialN);

                    N = potentialN;

                    state = READ_I;

                    break;
                }
                case READ_I:
                {
                    inputFileName.assign(argv[i]);
                    assert (inputFileName.size() < 100);
                    state = READ_O;
                    break;
                }
                case READ_O:
                {
                    outputFileName.assign(argv[i]);
                    assert (outputFileName.size() < 100);
                    state = END;
                    break;
                }
                case END: return;
                default:
                    ASRT_FAULT(isParseArgsUnknownState);
                }
            }
        }

    }args;

    struct sUrlStucture{
        int prefixBeginPosition;
        int domainBeginPosition;
        int pathBeginPosition;

        string prefix;
        string domain;        
        string path;

        bool isValid;

        sUrlStucture()
        {
            prefixBeginPosition = -1;
            domainBeginPosition = -1;
            pathBeginPosition = -1;
            prefix = "";
            domain = "";
            path = "";
            isValid = true;
        }
    };

    struct sUrlInfo{
        vector<string> CONTENT_PREFIX;
        string CONTENT_DOMAIN;
        string CONTENT_PATH;

        void appendSymbols(char first, char last, string &str)
        {
            size_t totalSymbols = last - first + 1;
            char curSymbol = first;
            string temp;
            temp.resize(totalSymbols);
            for (size_t i = 0; i < totalSymbols; ++i)
            {
                temp[i] = curSymbol + i;
            }
            str.append(temp);
        }

        sUrlInfo()
        {
            CONTENT_PREFIX.push_back("http://");
            CONTENT_PREFIX.push_back("https://");

            appendSymbols('a', 'z', CONTENT_DOMAIN);
            appendSymbols('A', 'Z', CONTENT_DOMAIN);
            appendSymbols('0', '9', CONTENT_DOMAIN);
            CONTENT_DOMAIN.append(".-");

            appendSymbols('a', 'z', CONTENT_PATH);
            appendSymbols('A', 'Z', CONTENT_PATH);
            appendSymbols('0', '9', CONTENT_PATH);
            CONTENT_PATH.append(".,/+_");
        }
    };

    struct sStatistics{

        int totalUrls;
        int totalDomains;
        int totalPaths;

        typedef pair<string, int> LexographicalPair;
        typedef map<LexographicalPair::first_type,
                    LexographicalPair::second_type> LexographicalSortedContainerType;
        typedef map<LexographicalPair::first_type,
                    LexographicalPair::second_type>::iterator LexographicalSortedContainerTypeIter;
        LexographicalSortedContainerType topDomainsLex;
        LexographicalSortedContainerType topPathsLex;

        typedef vector< LexographicalSortedContainerTypeIter > ByFreqSortedContainerType;
        typedef vector< LexographicalSortedContainerTypeIter >::iterator ByFreqSortedContainerTypeIter;
        ByFreqSortedContainerType topNDomains;
        ByFreqSortedContainerType topNPaths;

        sStatistics()
        {
            totalUrls = 0;
            totalDomains = 0;
            totalPaths = 0;
        }

        struct compare_by_freq {
            bool operator () (LexographicalSortedContainerTypeIter &lhs,
                              LexographicalSortedContainerTypeIter &rhs)
            {
                bool isFreqGreater = lhs->second > rhs->second;
                bool isFreqEqual = lhs->second == rhs->second;

                bool isNoSwap = true;

                if (isFreqGreater)
                {
                    isNoSwap = true;
                }
                else if (isFreqEqual)
                {
                    isNoSwap = lhs->first <= rhs->first;
                }
                else
                {
                    isNoSwap = false;
                }

                return isNoSwap;
            }
        };

        void fillTopContainerSortedByFreq(
                LexographicalSortedContainerType &lexSortedContainer,
                ByFreqSortedContainerType &byFreqSortedContainer)
        {
            LexographicalSortedContainerTypeIter itForLex =
                    lexSortedContainer.begin();
            LexographicalSortedContainerTypeIter itForLexEnd =
                    lexSortedContainer.end();

            byFreqSortedContainer.reserve(lexSortedContainer.size());

            for (; itForLex != itForLexEnd; ++itForLex)
            {
                byFreqSortedContainer.push_back(itForLex);
            }

            size_t firstNEls = args.N;
            if (args.N > byFreqSortedContainer.size())
            {
                firstNEls = byFreqSortedContainer.size();
            }

            partial_sort(byFreqSortedContainer.begin(),
                         byFreqSortedContainer.begin() + firstNEls,
                         byFreqSortedContainer.end(),
                         compare_by_freq());

            byFreqSortedContainer.resize(firstNEls);
            ByFreqSortedContainerType(byFreqSortedContainer).swap(byFreqSortedContainer);
        }

        void calcStatistics()
        {

            fillTopContainerSortedByFreq(topDomainsLex, topNDomains);
            fillTopContainerSortedByFreq(topPathsLex, topNPaths);

        }//calcStat

        void considerUrl(const sUrlStucture &url)
        {
            if (topDomainsLex.count(url.domain) == 0)
            {
                topDomainsLex.insert(make_pair(url.domain, 1));
                totalDomains += 1;
            }
            else
            {
                topDomainsLex[url.domain] += 1;
            }

            if (topPathsLex.count(url.path) == 0)
            {
                topPathsLex.insert(make_pair(url.path, 1));
                totalPaths += 1;
            }
            else
            {
                topPathsLex[url.path] += 1;
            }

            totalUrls += 1;
        }//consider url

    }statistics;

    struct sParsedLog{

        enum eStates{
            SEARCH_PREFIX,
            EXTRACT_DOMAIN,
            EXTRACT_PATH,
            END,
        };

        const sUrlInfo urlInfo;

        sParsedLog()
        {

        }


        string extractStringByContent(const string &lineForSearch,
                                      int beginCharIndex,
                                      const string &symbolsSet)
        {
            bool isSearchContinue = true;
            int charIndex = beginCharIndex;
            int endIndex = lineForSearch.size();

            string subString;
            subString.resize(lineForSearch.size());
            int subStringIndex = 0;

            while (isSearchContinue && (charIndex != endIndex))
            {
                char curChar = lineForSearch.at(charIndex);

                bool isCharFromSymbolsSet =
                        (symbolsSet.find(curChar) != string::npos);

                if (isCharFromSymbolsSet)
                {
                    subString[subStringIndex++] = curChar;
                }
                else
                {
                    isSearchContinue = false;
                }

                ++charIndex;
            }

            subString.resize(subStringIndex);

            return subString;
        }

        vector<size_t> findAllPrefixes(const string &lineForSearch,
                                        const string &prefix)
        {
            size_t foundPos = 0;
            vector<size_t> v;
            bool isFirstStep = true;
            do{

                foundPos = lineForSearch.find(prefix,
                                              isFirstStep ?
                                                  foundPos : foundPos+1);

                isFirstStep = false;

                if (foundPos != string::npos)
                {
                    v.push_back(foundPos);
                }

            }while(foundPos != string::npos);

            return v;
        }


        void parseInputFile(const string &inputFileName,
                            sStatistics &stat)
        {
            ifstream ifile;
            ifile.open(inputFileName.c_str());

            if (ifile.is_open())
            {
                string line;

                while (getline(ifile, line))
                {

                    eStates state = SEARCH_PREFIX;
                    deque<sUrlStucture> urlContainerPerLine;

                    bool isUrlSearchContinue = true;
                    while (isUrlSearchContinue)
                    {
                        switch (state) {
                        case SEARCH_PREFIX:
                        {
                            size_t prefixIndex = 0;
                            assert (urlInfo.CONTENT_PREFIX.size() >= 1);

                            do {
                                string checkedPrefix =
                                        urlInfo.CONTENT_PREFIX.at(prefixIndex);

                                vector<size_t> v = findAllPrefixes(line, checkedPrefix);

                                for (size_t posIndex = 0; posIndex < v.size(); ++posIndex)
                                {
                                    sUrlStucture urlStructure;
                                    urlStructure.prefixBeginPosition = v.at(posIndex);
                                    urlStructure.prefix = checkedPrefix;

                                    urlContainerPerLine.push_back(urlStructure);

                                    state = EXTRACT_DOMAIN;
                                }

                            } while (++prefixIndex < urlInfo.CONTENT_PREFIX.size());

                            bool isNoPrefixFound = (state == SEARCH_PREFIX);
                            if (isNoPrefixFound)
                            {
                                state = END;
                            }

                            break;
                        }
                        case EXTRACT_DOMAIN:
                        {
                            for (size_t i = 0; i < urlContainerPerLine.size(); ++i)
                            {
                                sUrlStucture &url = urlContainerPerLine[i];
                                url.domainBeginPosition =
                                        url.prefixBeginPosition +
                                        url.prefix.size();

                                url.domain = extractStringByContent(
                                            line, url.domainBeginPosition,
                                            urlInfo.CONTENT_DOMAIN);

                                if (url.domain.empty())
                                {
                                    url.isValid = false;
                                }
                            }

                            state = EXTRACT_PATH;

                            break;
                        }
                        case EXTRACT_PATH:
                        {
                            for (size_t i = 0; i < urlContainerPerLine.size(); ++i)
                            {
                                sUrlStucture &url = urlContainerPerLine[i];

                                if (! url.isValid)
                                {
                                    continue;
                                }

                                url.pathBeginPosition =
                                        url.domainBeginPosition +
                                        url.domain.size();

                                url.path = extractStringByContent(
                                            line, url.pathBeginPosition,
                                            urlInfo.CONTENT_PATH);

                                if (url.path.empty())
                                {
                                    url.path = "/";
                                }

                            }

                            state = END;
                            break;
                        }
                        case END:
                        {
                            for (size_t i = 0; i < urlContainerPerLine.size(); ++i)
                            {
                                sUrlStucture &url = urlContainerPerLine[i];
                                if (url.isValid)
                                {
                                    stat.considerUrl(url);
                                }
                            }
                            isUrlSearchContinue = false;
                            break;
                        }
                        default:
                            ASRT_FAULT(isParseInputFileUnknownState);
                        }

                    }//while search


                }// while getline


                ifile.close();
            }
            else
            {
                DEBUG2("can't open the file - ", inputFileName);
            }
        }

    }parsedLog;    


    struct sOutputFile{

        string toString(int num)
        {
            ostringstream convert;
            convert << num;
            return convert.str();
        }

        void writeOutputFileWithStatistics(
                const string &outFileName,
                const sStatistics &stat)
        {
            ofstream ofile;
            ofile.open(outFileName.c_str());

            if (ofile.is_open())
            {
                string totalString;

                totalString = "total urls ";
                totalString += toString(stat.totalUrls);
                totalString += ", domains ";
                totalString += toString(stat.totalDomains);
                totalString += ", paths ";
                totalString += toString(stat.totalPaths);
                totalString += "\n\n";

                string top_domains = "top domains\n";

                for (size_t i = 0; i < stat.topNDomains.size(); ++i)
                {
                    top_domains += toString(stat.topNDomains.at(i)->second);
                    top_domains += " ";
                    top_domains += stat.topNDomains.at(i)->first;
                    top_domains += "\n";
                }

                string top_paths = "\ntop paths\n";

                for (size_t i = 0; i < stat.topNPaths.size(); ++i)
                {
                    top_paths += toString(stat.topNPaths.at(i)->second);
                    top_paths += " ";
                    top_paths += stat.topNPaths.at(i)->first;
                    top_paths += "\n";
                }

                DEBUG(totalString);
                DEBUG(top_domains);
                DEBUG(top_paths);

                ofile << totalString;
                ofile << top_domains;
                ofile << top_paths;

                ofile.close();

            }
            else
            {
                DEBUG2("can't open the file - ", outFileName);
            }
        }

    }outFile;
}


int main(int argc, char *argv[])
{

    args.parseArgs(argv, argc);

    parsedLog.parseInputFile(args.inputFileName, statistics);

    statistics.calcStatistics();

    outFile.writeOutputFileWithStatistics(args.outputFileName, statistics);

    return 0;
}

