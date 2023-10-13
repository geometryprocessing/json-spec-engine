#include <iostream>
#include <cstdlib>
#include <jse/jse.h>
#include <string>
#include <vector>
#include <fstream>


using namespace std;
using namespace jse;

int main(int argc, char *argv[]) {
    try
    {

        if (argc < 3) {
            cerr << "This utility recursively replaces the include rules with their corresponding rule files." << endl;
            cerr << "Usage: " << argv[0] << " input.json output.json folder1 folder2 ..." << endl;
            return EXIT_FAILURE;
        }

        // First parameter is the input json
        string sinput = argv[1];

        // Second parameter is the output json
        string soutput = argv[2];

        // Other parameters are optional include directories
        // where the parser will look for rule files
        vector<std::string> arg;
        for (size_t i=3;i<argc;++i)
            arg.push_back(string(argv[i]));

        // Initialize JSE
        JSE jse;
        jse.include_directories = arg;

        // Load the input json
        std::ifstream input_f(sinput);
        json rules = json::parse(input_f);

        // Try to replace the includes, might throw exceptions
        json new_rules = jse.inject_include(rules);

        // Write the new rule file
        std::ofstream file(soutput);
        file << new_rules;

        return EXIT_SUCCESS;
    }
    catch(const std::exception &exc)
    {
        std::cout << exc.what() << std::endl;
        return EXIT_FAILURE;
    }

}