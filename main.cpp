#include <iostream>
#include <list>
#include <vector>
#include <cassert>

#include <string>
#include <sstream>
#include <fstream>
#include <queue>
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
                DEBUG_NM(argv[i]);

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
        string prefix;
        int prefixBeginPosition;
        string domain;
        int domainBeginPosition;
        string path;
        int pathBeginPosition;
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

    struct sParsedLog{

        enum eStates{
            SEARCH_PREFIX,
            EXTRACT_DOMAIN,
            EXTRACT_PATH,
            END,
        };

        list<sUrlStucture> urlContainer;

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
                    DEBUG_NM(line);

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

                            size_t foundPos = string::npos;
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


}



int main(int argc, char *argv[])
{

    args.parseArgs(argv, argc);

//    DEBUG_NM(args.N);
//    DEBUG_NM(args.inputFileName);
//    DEBUG_NM(args.outputFileName);

    parsedLog.parseInputFile(args.inputFileName);


    return 0;
}

