#ifndef PACKSTRINGTOHSITEM_HH_
#define PACKSTRINGTOHSITEM_HH_

#include <string>

int packStringToHSItem(int uid, const char* title, const char* category,
                       const std::string& str);

std::string unpackStringFromHSItem(int id);

#endif // PACKSTRINGTOHSITEM_HH_
