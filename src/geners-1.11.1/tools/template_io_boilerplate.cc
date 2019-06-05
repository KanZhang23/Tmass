#include <map>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "CmdLine.hh"

using namespace std;


static void print_usage(const char* progname)
{
    cout << "\nUsage: " << progname << " outputFile templateName Np p0_type p1_type ...\n\n"
         << "  outputFile   -- The name of the output header file.\n\n"
         << "  templateName -- The name of the template.\n\n"
         << "  Np           -- Number of template parameters affecting serialized\n"
         << "                  representation of the class. Specify some large number\n"
         << "                  here if all template parameters contribute.\n\n"
         << "  p0_type, etc -- Keywords indicating the kinds of template parameters used.\n"
         << "                  Possible keywords are \"class\", \"typename\", \"int\", \"unsigned\",\n"
         << "                  \"long\", \"unsigned long\", \"std::size_t\", and \"bool\".\n\n"
         << "Example: suppose, you have a template whose declaration looks like\n\n"
         << "template <typename Real, int N> MyTemplate { ... };\n\n"
         << "and N does not substantially affect the output format except through its\n"
         << "value known at the compile time. Then run this program as follows:\n\n"
         << progname << " MyTemplateIO.hh MyTemplate 1 typename int\n"
         << endl;
}


static std::string getFileExtension(const std::string& fileName)
{
    const unsigned long pos = fileName.find_last_of(".");
    if (pos != std::string::npos)
        return fileName.substr(pos + 1);
    else
        return "";
}


static std::string replaceChar(const std::string& in,
                               const char old,
                               const char newChar)
{
    std::string out(in);
    const unsigned sz = out.size();
    for (unsigned i=0; i<sz; ++i)
        if (out[i] == old)
            out[i] = newChar;
    return out;
}


int main(int argc, char const* argv[])
{
    CmdLine cmdline(argc, argv);
    if (argc == 1)
    {
        print_usage(cmdline.progname());
        return 0;
    }

    std::map<std::string,std::pair<int,char> > keywords;
    keywords["class"]         = std::make_pair(1, 'C');
    keywords["typename"]      = std::make_pair(2, 'T');
    keywords["bool"]          = std::make_pair(3, 'B');
    keywords["int"]           = std::make_pair(4, 'N');
    keywords["unsigned"]      = std::make_pair(5, 'N');
    keywords["long"]          = std::make_pair(6, 'N');
    keywords["unsigned long"] = std::make_pair(7, 'N');
    keywords["std::size_t"]   = std::make_pair(8, 'N');

    std::string outputFile, templateName;
    std::vector<std::string> params;
    unsigned nP;

    try {
        cmdline.optend();

        const unsigned cmdargc = cmdline.argc();
        if (cmdargc < 4)
            throw CmdLineError("wrong number of command line arguments");
        cmdline >> outputFile >> templateName >> nP;
        const unsigned n = cmdargc - 3;
        params.resize(n);
        for (unsigned i=0; i<n; ++i)
        {
            cmdline >> params[i];
            if (!keywords[params[i]].first)
                throw CmdLineError() << "invalid keyword \"" << params[i] << "\"";
        }
    }
    catch (CmdLineError& e) {
        cerr << "Error in " << cmdline.progname() << ": "
             << e.str() << endl;
        print_usage(cmdline.progname());
        return 1;
    }

    ofstream of(outputFile.c_str());
    std::string headerGuard = replaceChar(outputFile, '/', '_');
    headerGuard = replaceChar(headerGuard, '.', '_');
    headerGuard += '_';
    std::transform(headerGuard.begin(), headerGuard.end(),
                   headerGuard.begin(), ::toupper);
    of << "#ifndef " << headerGuard << '\n'
       << "#define " << headerGuard << "\n\n"
       << "// VERIFY / UPDATE THE NAME OF THE FOLLOWING HEADER\n"
       << "#include \"" << templateName << "."
       << getFileExtension(outputFile) << "\"\n\n"
       << "#include \"geners/GenericIO.hh\"\n";

    if (std::find(params.begin(), params.end(), std::string("bool")) != params.end())
        of << "#include \"geners/BooleanString.hh\"\n\n";
    else
        of << '\n';

    const std::string& suffix = replaceChar(templateName, ':', '_');
    const unsigned nTParams = params.size();

    // The standard template args
    std::string parameterSequence;
    {
        ostringstream os;
        for (unsigned i=0; i<nTParams; ++i)
        {
            if (i) os << ',';
            os << params[i] << ' ' << keywords[params[i]].second << i;
        }
        parameterSequence = os.str();
    }

    std::string templateArgs = "template<";
    templateArgs += parameterSequence;
    templateArgs += '>';

    std::string templateArgVals;
    {
        ostringstream os;
        os << '<';
        for (unsigned i=0; i<nTParams; ++i)
        {
            if (i) os << ',';
            os << keywords[params[i]].second << i;
        }
        os << '>';
        templateArgVals = os.str();
    }

    of << "namespace gs {\n"
       << "    //\n"
       << "    // Let the system know that the serialization is performed\n"
       << "    // by a user-defined external mechanism\n"
       << "    //\n"
       << "    " << templateArgs << '\n'
       << "    struct IOIsExternal<" 
       << templateName << templateArgVals << " >\n"
       << "       {enum {value = 1};};\n"
       << "    " << templateArgs << '\n'
       << "    struct IOIsExternal<const " 
       << templateName << templateArgVals << " >\n"
       << "       {enum {value = 1};};\n"
       << "    " << templateArgs << '\n'
       << "    struct IOIsExternal<volatile " 
       << templateName << templateArgVals << " >\n"
       << "       {enum {value = 1};};\n"
       << "    " << templateArgs << '\n'
       << "    struct IOIsExternal<const volatile " 
       << templateName << templateArgVals << " >\n"
       << "       {enum {value = 1};};\n\n";

    of << "    " << templateArgs << '\n'
       << "    inline std::string template_class_name_" << suffix
       << "(\n        const std::string& tN, const unsigned nIncl)\n"
       << "    {\n        return template_class_name_" << suffix
       << templateArgVals << "(tN.c_str(), nIncl);\n    }\n\n";

    of << "    //\n"
       << "    // The following function generates the template class name\n"
       << "    // for serialization purposes\n"
       << "    //\n"
       << "    " << templateArgs << '\n'
       << "    inline std::string template_class_name_" << suffix
       << "(\n        const char* templateName, const unsigned nIncl)\n"
       << "    {\n"
       << "        assert(templateName);\n"
       << "        std::string name(templateName);\n"
       << "        if (nIncl)\n"
       << "        {\n"
       << "            name += '<';\n";

    for (unsigned i=0; i<nTParams; ++i)
    {
        if (i)
            of << "            if (nIncl > " << i << ")\n";
        of << "            {\n";
        if (i)
            of << "                name += ',';\n";
        if (keywords[params[i]].first <= 2)
        {
            of << "                const ClassId& id1(ClassIdSpecialization<"
               << keywords[params[i]].second
               << i << ">::classId());\n"
               << "                name += id1.id();\n";
        }
        else if (keywords[params[i]].second == 'B')
        {
            of << "                name += BooleanString<B" << i << ">::name();\n"
               << "                name += \"(0)\";\n";
        }
        else
        {
            of << "                std::ostringstream os;\n"
               << "                os << "
               << keywords[params[i]].second << i << " << \"(0)\";\n"
               << "                name += os.str();\n";
        }
        of << "            }\n";
    }

    of << "            name += '>';\n"
       << "        }\n"
       << "        return name;\n"
       << "    }\n}\n\n";

    std::string macroName = "gs_specialize_template_hlp_";
    macroName += suffix;

    of << "#define " << macroName
       << "(qualifyer, name, version, MAX) /**/ \\\n"
       << "    " << templateArgs << " \\\n"
       << "    struct ClassIdSpecialization<qualifyer name " 
       << templateArgVals << " > \\\n"
       << "    {inline static ClassId classId(const bool isPtr=false) \\\n"
       << "    {return ClassId(template_class_name_" 
       << suffix << templateArgVals << "(#name,MAX), version, isPtr);}};\n\n";

    of << "#define gs_specialize_template_id_" << suffix << "(name, version, MAX) /**/ \\\n"
       << "namespace gs { \\\n"
       << "    " << macroName << "(GENERS_EMPTY_TYPE_QUALIFYER_, name, version, MAX) \\\n"
       << "    " << macroName << "(const, name, version, MAX) \\\n"
       << "    " << macroName << "(volatile, name, version, MAX) \\\n"
       << "    " << macroName << "(const volatile, name, version, MAX) \\\n"
       << "}\n\n";
    
    of << "//\n"
       << "// Actual specialization of the template class id\n"
       << "//\n"
       << "gs_specialize_template_id_" << suffix << "("
       << templateName << ", 1, " << nP << ")\n\n";

    of << "//\n"
       << "// Specialize the behavior of the two template classes at the heart of\n"
       << "// the serialization facility: gs::GenericWriter and gs::GenericReader\n"
       << "//\n"
       << "namespace gs {\n"
       << "    template <class Stream,class State," << parameterSequence << ">\n"
       << "    struct GenericWriter<Stream, State, " << templateName << templateArgVals << ",\n"
       << "                         Int2Type<IOTraits<int>::ISEXTERNAL> >\n"
       << "    {\n"
       << "        inline static bool process(const " << templateName << templateArgVals << "& s, Stream& os,\n"
       << "                                   State*, const bool processClassId)\n"
       << "        {\n"
       << "            // If necessary, serialize the class id\n"
       << "            static const ClassId current(ClassId::makeId<" << templateName << templateArgVals << " >());\n"
       << "            bool status = processClassId ? current.write(os) : true;\n"
       << '\n'
       << "            // Serialize object data if the class id was successfully written out\n"
       << "            if (status)\n"
       << "            {\n"
       << "                //*********************************************************\n"
       << "                // THE CODE SERIALIZING TEMPLATE INFO BELONGS IN THIS SCOPE\n"
       << "                //*********************************************************\n"
       << "                //\n"
       << "                // write_item(os, s.getSomething());\n"
       << "                // write_item(os, s.getSomethingElse());\n"
       << "            }\n"
       << '\n'
       << "            // Return \"true\" on success, \"false\" on failure\n"
       << "            return status && !os.fail();\n"
       << "        }\n"
       << "    };\n"
       << '\n'
       << "    template <class Stream,class State," << parameterSequence << ">\n"
       << "    struct GenericReader<Stream, State, " << templateName << templateArgVals << ",\n"
       << "                         Int2Type<IOTraits<int>::ISEXTERNAL> >\n"
       << "    {\n"
       << "        inline static bool readIntoPtr(" << templateName << templateArgVals << "*& ptr, Stream& is,\n"
       << "                                       State* st, const bool processClassId)\n"
       << "        {\n"
       << "            // Make sure that the serialized class id is consistent with\n"
       << "            // the current one\n"
       << "            static const ClassId current(ClassId::makeId<" << templateName << templateArgVals << " >());\n"
       << "            const ClassId& stored = processClassId ? ClassId(is,1) : st->back();\n"
       << '\n'
       << "            // Check that the name is consistent. Do not check for the\n"
       << "            // consistency of the complete id because we want to be able\n"
       << "            // to read different versions of parameter classes.\n"
       << "            current.ensureSameName(stored);\n"
       << '\n'
       << "            // ******************************************************\n"
       << "            // PUT THE CODE READING CONSTRUCTOR ARGUMENTS RIGHT BELOW\n"
       << "            // ******************************************************\n"
       << "            //\n"
       << "            // CPP11_auto_ptr<T0> read0 = read_item<T0>(is);\n"
       << '\n'
       << "            if (ptr == 0)\n"
       << "                ptr = new " << templateName << templateArgVals << "(/* CONSTRUCTOR ARGUMENTS HERE */);\n"
       << "            else\n"
       << "                *ptr = " << templateName << templateArgVals << "(/* CONSTRUCTOR ARGUMENTS HERE */);\n"
       << "            return true;\n"
       << "        }\n"
       << '\n'
       << "        inline static bool process(" << templateName << templateArgVals << "& s, Stream& is,\n"
       << "                                   State* st, const bool processClassId)\n"
       << "        {\n"
       << "            // Simply convert reading by reference into reading by pointer\n"
       << "            " << templateName << templateArgVals << "* ps = &s;\n"
       << "            return readIntoPtr(ps, is, st, processClassId);\n"
       << "        }\n"
       << "    };\n"
       << "}\n\n";

    of << "#endif // not " << headerGuard << '\n';
    return 0;
}
