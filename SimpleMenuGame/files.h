#pragma once

#include "text.h"

/* The XML file is re-read every time a new command is issued. */
bool ReadFile(bool firstBoot = false);
