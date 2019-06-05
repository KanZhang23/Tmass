#include <cstdio>
#include "npstat/stat/AbsNtuple.hh"

namespace npstat {
    std::vector<std::string> ntupleColumns(const char* v0)
    {
        std::vector<std::string> vars;
        vars.reserve(1);
        assert(v0); vars.push_back(std::string(v0));
        return vars;
    }

    std::vector<std::string> ntupleColumns(const char* v0, const char* v1)
    {
        std::vector<std::string> vars;
        vars.reserve(2);
        assert(v0); vars.push_back(std::string(v0));
        assert(v1); vars.push_back(std::string(v1));
        return vars;
    }

    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2)
    {
        std::vector<std::string> vars;
        vars.reserve(3);
        assert(v0); vars.push_back(std::string(v0));
        assert(v1); vars.push_back(std::string(v1));
        assert(v2); vars.push_back(std::string(v2));
        return vars;
    }
    
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3)
    {
        std::vector<std::string> vars;
        vars.reserve(4);
        assert(v0); vars.push_back(std::string(v0));
        assert(v1); vars.push_back(std::string(v1));
        assert(v2); vars.push_back(std::string(v2));
        assert(v3); vars.push_back(std::string(v3));
        return vars;
    }
    
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4)
    {
        std::vector<std::string> vars;
        vars.reserve(5);
        assert(v0); vars.push_back(std::string(v0));
        assert(v1); vars.push_back(std::string(v1));
        assert(v2); vars.push_back(std::string(v2));
        assert(v3); vars.push_back(std::string(v3));
        assert(v4); vars.push_back(std::string(v4));
        return vars;
    }
    
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5)
    {
        std::vector<std::string> vars;
        vars.reserve(6);
        assert(v0); vars.push_back(std::string(v0));
        assert(v1); vars.push_back(std::string(v1));
        assert(v2); vars.push_back(std::string(v2));
        assert(v3); vars.push_back(std::string(v3));
        assert(v4); vars.push_back(std::string(v4));
        assert(v5); vars.push_back(std::string(v5));
        return vars;
    }
    
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5,
                                           const char* v6)
    {
        std::vector<std::string> vars;
        vars.reserve(7);
        assert(v0); vars.push_back(std::string(v0));
        assert(v1); vars.push_back(std::string(v1));
        assert(v2); vars.push_back(std::string(v2));
        assert(v3); vars.push_back(std::string(v3));
        assert(v4); vars.push_back(std::string(v4));
        assert(v5); vars.push_back(std::string(v5));
        assert(v6); vars.push_back(std::string(v6));
        return vars;
    }
    
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5,
                                           const char* v6, const char* v7)
    {
        std::vector<std::string> vars;
        vars.reserve(8);
        assert(v0); vars.push_back(std::string(v0));
        assert(v1); vars.push_back(std::string(v1));
        assert(v2); vars.push_back(std::string(v2));
        assert(v3); vars.push_back(std::string(v3));
        assert(v4); vars.push_back(std::string(v4));
        assert(v5); vars.push_back(std::string(v5));
        assert(v6); vars.push_back(std::string(v6));
        assert(v7); vars.push_back(std::string(v7));
        return vars;
    }
    
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5,
                                           const char* v6, const char* v7,
                                           const char* v8)
    {
        std::vector<std::string> vars;
        vars.reserve(9);
        assert(v0); vars.push_back(std::string(v0));
        assert(v1); vars.push_back(std::string(v1));
        assert(v2); vars.push_back(std::string(v2));
        assert(v3); vars.push_back(std::string(v3));
        assert(v4); vars.push_back(std::string(v4));
        assert(v5); vars.push_back(std::string(v5));
        assert(v6); vars.push_back(std::string(v6));
        assert(v7); vars.push_back(std::string(v7));
        assert(v8); vars.push_back(std::string(v8));
        return vars;
    }
    
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5,
                                           const char* v6, const char* v7,
                                           const char* v8, const char* v9)
    {
        std::vector<std::string> vars;
        vars.reserve(10);
        assert(v0); vars.push_back(std::string(v0));
        assert(v1); vars.push_back(std::string(v1));
        assert(v2); vars.push_back(std::string(v2));
        assert(v3); vars.push_back(std::string(v3));
        assert(v4); vars.push_back(std::string(v4));
        assert(v5); vars.push_back(std::string(v5));
        assert(v6); vars.push_back(std::string(v6));
        assert(v7); vars.push_back(std::string(v7));
        assert(v8); vars.push_back(std::string(v8));
        assert(v9); vars.push_back(std::string(v9));
        return vars;
    }

    std::vector<std::string> ntupleColumns(const char** names,
                                           const unsigned len)
    {
        std::vector<std::string> vars;
        vars.reserve(len);
        for (unsigned i=0; i<len; ++i)
        {
            assert(names[i]);
            vars.push_back(names[i]);
        }
        return vars;
    }

    std::vector<std::string> simpleColumnNames(const unsigned len)
    {
        char buf[32];
        std::vector<std::string> vars;
        vars.reserve(len);
        for (unsigned i=0; i<len; ++i)
        {
            sprintf(buf, "c%u", i);
            vars.push_back(buf);
        }
        return vars;
    }
}
