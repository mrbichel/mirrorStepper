#ifndef PARSER_H
#define PARSER_H

#include "Arduino.h"

namespace parser {
    String peekToken();

    String nextToken();

    bool nextTokenIf(const String& other_);

    bool done();

    // Read a new message.
    bool read(int timeout_);
}

#endif
