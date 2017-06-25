#include <iostream>
#include <list>
#include <vector>
#include <cassert>

#include <string>
#include <sstream>
#include <fstream>
#include <queue>
#include <set>
#include <map>
using namespace std;

namespace {

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

        int N;
        string inputFileName;
        string outputFileName;
        sArgs()
        {
            N = -1;
            inputFileName = "no";
            outputFileName = "no";
        }

        void parseArgs(char *argv[], int argc)
        {
            eParseArgsState state = SEARCH_N;
            for (int i = 1; i < argc; ++i)
            {
//                DEBUG_NM(argv[i]);

                switch (state) {
                case SEARCH_N:
                {
                    if (argv[i] == string("-n"))
                    {
                        state = READ_N;
                    }
                    else
                    {
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

        sUrlStucture()
        {
            prefixBeginPosition = -1;
            domainBeginPosition = -1;
            pathBeginPosition = -1;
            prefix = "";
            domain = "";
            path = "";
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

//            DEBUG_NM(CONTENT_PREFIX[0]);
//            DEBUG_NM(CONTENT_PREFIX[1]);
//            DEBUG_NM(CONTENT_DOMAIN);
//            DEBUG_NM(CONTENT_PATH);
        }
    };

    typedef list<sUrlStucture> UrlContainerType;
    typedef list<sUrlStucture>::const_iterator UrlContainerTypeConstIter;
    struct sParsedLog{

        enum eStates{
            SEARCH_PREFIX,
            EXTRACT_DOMAIN,
            EXTRACT_PATH,
            END,
        };

        UrlContainerType urlContainer;

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
            int lastIndex = lineForSearch.size() - 1;

            string subString;
            subString.resize(lineForSearch.size());
            int subStringIndex = 0;

            while (isSearchContinue && (charIndex != lastIndex))
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
            do{

                foundPos = lineForSearch.find(prefix, foundPos+1);
                if (foundPos != string::npos)
                {
                    v.push_back(foundPos);
                }

            }while(foundPos != string::npos);

            return v;
        }

        void parseInputFile(const string &inputFileName)
        {
            ifstream ifile;
            ifile.open(inputFileName.c_str());

            if (ifile.is_open())
            {
                string line;

                while (getline(ifile, line))
                {
//                    DEBUG_NM(line);

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

                                state = EXTRACT_PATH;
                            }
                            break;
                        }
                        case EXTRACT_PATH:
                        {
                            for (size_t i = 0; i < urlContainerPerLine.size(); ++i)
                            {
                                sUrlStucture &url = urlContainerPerLine[i];
                                url.pathBeginPosition =
                                        url.domainBeginPosition +
                                        url.domain.size();

                                url.path = extractStringByContent(
                                            line, url.pathBeginPosition,
                                            urlInfo.CONTENT_PATH);

                                state = END;
                            }
                            break;
                        }
                        case END:
                        {
                            for (size_t i = 0; i < urlContainerPerLine.size(); ++i)
                            {
                                urlContainer.push_back(urlContainerPerLine[i]);
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

    struct sStatistics{

        int totalUrls;
        int totalDomains;
        int totalPaths;

        typedef pair<string, int> LexographicalPair;
        typedef map<LexographicalPair> LexographicalSortedContainerType;
        typedef map<LexographicalPair>::iterator LexographicalSortedContainerTypeIter;
        LexographicalSortedContainerType topDomainsLex;
        LexographicalSortedContainerType topPathsLex;

        typedef pair<int, string> ByFreqPair;
        typedef vector< ByFreqPair > ByFreqSortedContainerType;
        typedef vector< ByFreqPair >::iterator ByFreqSortedContainerTypeIter;
        ByFreqSortedContainerType topDomains;
        ByFreqSortedContainerType topPaths;

        sStatistics()
        {
            totalUrls = 0;
            totalDomains = 0;
            totalPaths = 0;
        }

        bool compare_by_freq (const ByFreqPair& first,
                              const ByFreqPair& second)
        {
            return ( first.first > second.first );
        }

        void fillTopContainerSortedByFreq(
                LexographicalSortedContainerType &lexSortedContainer,
                ByFreqSortedContainerType &byFreqSortedContainer)
        {
            LexographicalSortedContainerTypeIter itForMap =
                    lexSortedContainer.begin();
            LexographicalSortedContainerTypeIter itForMapEnd =
                    lexSortedContainer.end();
            for (; itForMap != itForMapEnd; ++itForMap)
            {
                byFreqSortedContainer.push_back(
                            make_pair(itForMap->second, itForMap->first));
            }

            byFreqSortedContainer.sort(compare_by_freq());

            args.N
        }

        void calcStatistics(const UrlContainerType &urlContainer)
        {
            UrlContainerTypeConstIter itForUrl = urlContainer.begin();
            UrlContainerTypeConstIter itForUrlEnd = urlContainer.end();
            for (; itForUrl != itForUrlEnd; ++itForUrl) {
//                DEBUG_NM(it->domain);

                if (topDomainsLex.count(itForUrl->domain) == 0)
                {
                    topDomainsLex.insert(make_pair(itForUrl->domain, 1));
                    totalDomains += 1;
                }
                else
                {
                    topDomainsLex[itForUrl->domain] += 1;
                }

                if (topPathsLex.count(itForUrl->path) == 0)
                {
                    topPathsLex.insert(make_pair(itForUrl->path, 1));
                    totalPaths += 1;
                }
                else
                {
                    topPathsLex[itForUrl->path] += 1;
                }

                totalUrls += 1;

             }//for

            fillTopContainerSortedByFreq(topDomainsLex, topDomains);
            fillTopContainerSortedByFreq(topPathsLex, topPaths);

        }//calcStat

    }statistics;


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

//                DEBUG_NM(totalString);

                string top_domains = "top domains\n";

// consider N !!!!

//                total urls 5, domains 2, paths 5

//                top domains
//                4 en.wikipedia.org
//                1 www.google.com

//                top paths
//                1 /search
//                1 /w/index.php
//                1 /wiki/Free_software
//                1 /wiki/Kirschkuchen
//                1 /wiki/Main_Page

//                while (getline(ofile, line))
//                {

//                }// while getline


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

//    DEBUG_NM(args.N);
//    DEBUG_NM(args.inputFileName);
//    DEBUG_NM(args.outputFileName);

    parsedLog.parseInputFile(args.inputFileName);

    statistics.calcStatistics(parsedLog.urlContainer);

    outFile.writeOutputFileWithStatistics(args.outputFileName, statistics);

    return 0;
}

