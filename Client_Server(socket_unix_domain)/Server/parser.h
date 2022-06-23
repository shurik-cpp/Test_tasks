#pragma once

#include <iostream>
#include <algorithm>
#include "handler.h"

std::vector<std::string> SplitIntoWords(const std::string& str);
Handler ParseRequest(const std::string s);

