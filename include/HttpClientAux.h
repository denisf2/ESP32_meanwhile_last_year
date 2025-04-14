#ifndef HTTPCLIENTAUX_H__
#define HTTPCLIENTAUX_H__

#include <optional>

#include "WString.h"

auto SendGetRequest(const String& aUrl) -> std::optional<String>;

#endif