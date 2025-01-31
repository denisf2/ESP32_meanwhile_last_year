#ifndef TLOG_H_
#define TLOG_H_

#include <stddef.h>
#include <HardwareSerial.h>

namespace TLog
{
  template <typename T>
  auto print(T&& aArg) -> size_t
  {
    return Serial.print(aArg);
  }

  template <typename T>
  auto println(T&& aArg) -> size_t 
  {
    auto size = Serial.println();
    size += Serial.print(millis());
    size += Serial.print(" ");
    size += Serial.print(aArg);
    return size;
  }

  auto println() -> size_t;
}

#endif /* TLOG_H_ */