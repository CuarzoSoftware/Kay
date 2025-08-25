#include <CZ/AK/AKLog.h>

using namespace CZ;

const CZ::CZLogger &AKLogger() noexcept
{
    static CZLogger logger { "Kay", "CZ_KAY_LOG_LEVEL" };
    return logger;
}
