#include <string>
#include <cstring>
#include <cassert>
#include <sstream>

#include "loadResultsFromHS.hh"
#include "packStringToHSItem.hh"

#include "hsTypes.h"
#include "histoscope.h"

#include "geners/GenericIO.hh"

void loadResultsFromHS(const char* filename, const char* category,
                       std::vector<JesIntegResult>* results)
{
    const char prefix[] = "T5pLe_VelD0mMy1_6Qs18j9x";

    assert(filename);
    assert(category);
    assert(results);

    const int olditems = hs_num_items();
    const int nread = hs_read_file(filename, prefix);
    if (nread <= 0)
        return;

    std::string dir(prefix);
    if (strlen(category) > 0)
    {
        dir += '/';
        dir += category;
    }

    // Get the list of items
    const int nitems = hs_num_items();
    std::vector<int> idlist(nitems);
    const int nlist = hs_list_items("", "...", &idlist[0], nitems, 1);
    assert(nlist == nitems);

    // Buffers needed for subsequent cycling
    const unsigned catlen = HS_MAX_CATEGORY_LENGTH + 1;
    char catbuf[catlen] = {'\0',};
    JesIntegResult res;

    // Cycle over the items
    for (int i=0; i<nitems; ++i)
    {
        const int id = idlist[i];
        if (hs_type(id) == HS_NTUPLE)
            if (hs_num_variables(id) == 1)
            {
                hs_category(id, catbuf);
                if (strncmp(catbuf, dir.c_str(), catlen) == 0)
                {
                    std::istringstream is(unpackStringFromHSItem(id));
                    gs::restore_item(is, &res);
                    res.uid = hs_uid(id);
                    results->push_back(res);
                }
            }
    }

    std::string todel(prefix);
    todel += "/...";
    hs_delete_category(todel.c_str());
    assert(olditems == hs_num_items());
}
