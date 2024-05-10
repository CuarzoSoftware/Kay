#ifndef AKAPPLICATION_H
#define AKAPPLICATION_H

#include <AK.h>
#include <memory>

class AK::AKApplication
{
public:
    AKApplication();
    ~AKApplication();

    int run();

    class Private;

    Private *imp() const
    {
        return m_imp.get();
    }
private:
    std::unique_ptr<Private> m_imp;
};

#endif // AKAPPLICATION_H
