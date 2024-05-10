#ifndef AKOBJECT_H
#define AKOBJECT_H

#include <AK.h>
#include <assert.h>

class AK::AKObject
{
public:
    AKObject()
    {
        assert(app() != nullptr && "AKObjects can't be created without an AKApplication.");
    }

    virtual ~AKObject()
    {

    }
};

#endif // AKOBJECT_H
