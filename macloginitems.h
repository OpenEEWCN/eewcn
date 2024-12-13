#ifndef MACLOGINITEMS_H
#define MACLOGINITEMS_H

class MacLoginItems
{
public:
    bool isAutorun(const char* appPath);
    void setAutorun(bool enable,const char* appPath);
};

#endif // MACLOGINITEMS_H
