#include <TLog.h>

namespace TLog
{
    auto print(double aArg, int digits) -> size_t
    {
        return Serial.print(aArg, digits);
    }

    auto println() -> size_t
    {
        return Serial.println();
    }
}