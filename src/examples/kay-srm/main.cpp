#include <Application.h>
#include <CZ/Core/CZTimer.h>

using namespace CZ;

int main(void)
{
    setenv("SRM_DEBUG", "3", 0);
    setenv("KAY_DEBUG", "3", 0);

    Application app;
    CZTimer::OneShot(10000, [&app](CZTimer*){ app.finish(); });
    return app.exec();
}
