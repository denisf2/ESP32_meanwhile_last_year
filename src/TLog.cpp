#include <TLog.h>

namespace TLog
{
    auto println() -> size_t
    {
        return Serial.println();
    }
}