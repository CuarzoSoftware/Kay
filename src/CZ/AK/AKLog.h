#ifndef CZ_AKLOG_H
#define CZ_AKLOG_H

#include <CZ/Core/CZLogger.h>

/**
 * @brief Kay's logger, e.g. `AKLog(CZError, "Missing target: {}", name)`.
 * @ingroup AKCore
 */
#define AKLog AKLogger()

/**
 * @brief Returns Kay's CZLogger instance (namespace "Kay").
 *
 * The level is read from the `CZ_KAY_LOG_LEVEL` environment variable
 * and can be changed at runtime with CZLogger::setLevel().
 * Usually invoked through the AKLog macro.
 *
 * @ingroup AKCore
 */
const CZ::CZLogger &AKLogger() noexcept;

#endif // CZ_AKLOG_H

