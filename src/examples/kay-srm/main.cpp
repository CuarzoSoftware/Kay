#include <Application.h>
#include <AK/AKTimer.h>

using namespace AK;

int main(void)
{
    setenv("SRM_DEBUG", "3", 0);
    setenv("KAY_DEBUG", "3", 0);

    Application app;
    AKTimer::OneShoot(10000, [&app](AKTimer*){ app.finish(); });
    return app.exec();
}
