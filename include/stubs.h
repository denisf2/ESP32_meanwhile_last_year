#pragma once
// #include <OneWire.h>
// #include <DallasTemperature.h>

// ---------------------------------------------------
// dummy stub
// ---------------------------------------------------
#define ONE_WIRE_BUS 23

class Dummy_OneWire
{
public:
  Dummy_OneWire(int _tmp) {}
};

class Dummy_DallasTemperature
{
private:
  mutable unsigned int m_value{0};

public:
  Dummy_DallasTemperature() = default;
  Dummy_DallasTemperature(const Dummy_OneWire *const _tmp) {}
  void requestTemperatures() const {}
  int getTempCByIndex(uint _aIndex) const
  {
    m_value = std::min(m_value + 5, 100U);
    return m_value;
  }
  void begin() {}
};