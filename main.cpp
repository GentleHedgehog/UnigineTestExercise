#include <iostream>
#include <list>
#include <vector>
#include <cassert>

#include <string>
#include <sstream>
#include <fstream>
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
        string domain;
        string path;
        sUrlStucture()
        {
            prefix = "no";
            domain = "no";
            path = "no";
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
            for (char i = 0; i < totalSymbols; ++i)
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

            DEBUG_NM(CONTENT_PREFIX[0]);
            DEBUG_NM(CONTENT_PREFIX[1]);
            DEBUG_NM(CONTENT_DOMAIN);
            DEBUG_NM(CONTENT_PATH);
        }
    };

    struct sParsedLog{

        enum eStates{
            SEARCH_PREFIX,
            EXTRACT_DOMAIN,
            EXTRACT_PATH,
        };

        list<sUrlStucture> urlContainer;

        const sUrlInfo urlInfo;

        sParsedLog()
        {

        }



        void parseInputFile(const string &inputFileName)
        {
            ifstream ifile;
            ifile.open(inputFileName.c_str());

            if (ifile.is_open())
            {
                string line;
                eStates state = SEARCH_PREFIX;

                while (getline(ifile, line))
                {
                    DEBUG_NM(line);

                    int prefixPos = line.find();

                    while ()

                    switch (state) {
                    case SEARCH_PREFIX:
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


                ifile.close();
            }
            else
            {
                DEBUG2("can't open the file - ", inputFileName);
            }
        }
        }

    }parsedLog;


}



int main(int argc, char *argv[])
{

    args.parseArgs(argv, argc);

    DEBUG_NM(args.N);
    DEBUG_NM(args.inputFileName);
    DEBUG_NM(args.outputFileName);

//    parsedLog.parseInputFile(args.inputFileName);


    return 0;
}

