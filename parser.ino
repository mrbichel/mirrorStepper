#include "parser.h"

namespace parser {

const char delimiter_msg = '\n';
const char delimiter_tok = ' ';

// Message helpers ---------------------------------------------------------- //

// Return the position of the first token delimiter in 'message_', starting
// 'beg_', or the end of 'message_' if there are no further token delimiters.
unsigned char findTokenEnd(const String& message_, unsigned char beg_) {
    char result = message_.indexOf(delimiter_tok, beg_);
    if (result == -1) {
        result = message_.length();
    }
    return result;
}

// True if 'message_' is empty, false otherwise.
bool messageIsEmpty(const String& message_) {
    return message_.length() == 0;
}

// Message state ------------------------------------------------------------ //

String message;
unsigned char token_beg;
unsigned char token_end;
bool done_;

// Message management ------------------------------------------------------- //

// Reset the message state with 'message_'.
void setMessage(const String& message_) {
    message = message_;
    token_beg = 0;
    token_end = findTokenEnd(message, 0);
    done_ = messageIsEmpty(message);
}

// Advance the message state to the next token.
void advanceToken() {
    if (token_end == message.length()) {
        done_ = true;
    } else {
        token_beg = token_end + 1;
        token_end = findTokenEnd(message, token_beg);
    }
}

// Token interface ---------------------------------------------------------- //

String peekToken() {
    return message.substring(token_beg, token_end);
}

String nextToken() {
    String result = peekToken();
    advanceToken();
    return result;
}

bool nextTokenIf(const String& other_) {
    if (peekToken() == other_) {
        advanceToken();
        return true;
    } else {
        return false;
    }
}

bool done() {
    return done_;
}

// Serial communication interface ------------------------------------------- //

bool read(int timeout_) {
    const unsigned long start = millis();
    String new_message = "";

    do {
        int b = Serial.read();

        if (b == -1) {
            // If nothing was read and we are not in the middle of a message emit
            // an empty message.
            if (new_message.length() == 0) {
                setMessage("");
                return false;
            }
            // If nothing was read and we are in the middle of a message, wait a
            // bit and try again.
            else if (timeout_ > 0) {
                delay(1);
            }
        }
        // If we read a message delimiter, the message is done so return it.
        else if (b == delimiter_msg) {
            setMessage(new_message);
            return !messageIsEmpty(new_message);
        }
        // All other characters are appended to the message in progress.
        else {
            new_message += (char)b;
        }
    } while (millis() - start < timeout_);

    // On timeout, emit an empty message.
    setMessage("");
    return false;
}
}

