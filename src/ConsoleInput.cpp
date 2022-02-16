/* MIT License - Copyright (c) 2020 Francis Van Roie francis@netwize.be
   For full license information read the LICENSE file in the project folder */

#include "ConsoleInput.h"

// Definitions
const int ConsoleInput::KEY_NONE;
const int ConsoleInput::KEY_UNKNOWN;

const int ConsoleInput::KEY_BACKSPACE;
const int ConsoleInput::KEY_LF;
const int ConsoleInput::KEY_CR;
const int ConsoleInput::KEY_FN;

const int ConsoleInput::KEY_UP;
const int ConsoleInput::KEY_DOWN;
const int ConsoleInput::KEY_LEFT;
const int ConsoleInput::KEY_RIGHT;

const int ConsoleInput::KEY_PAGE_UP;
const int ConsoleInput::KEY_PAGE_DOWN;
const int ConsoleInput::KEY_INSERT;
const int ConsoleInput::KEY_DELETE;
const int ConsoleInput::KEY_HOME;
const int ConsoleInput::KEY_END;

const int ConsoleInput::MOD_SHIFT;
const int ConsoleInput::MOD_CTRL;
const int ConsoleInput::MOD_CMND;
const int ConsoleInput::MOD_ALT;
const int ConsoleInput::MOD_ALT_GR;

// ======== Constructors =======================

ConsoleInput::ConsoleInput(Stream *serial, size_t size)
{
    stream = serial;

    line_cb = NULL;
    flags.insert_mode = true;
    flags.debug_mode = false;
    flags.auto_update = true;
    flags.auto_edit = true;
    flags.auto_move = true;
    flags.auto_clear = true;
    flags.auto_history = true;
    flags.hide_password = false;
    history_index = 0;
    caret_pos = 0;
    last_read = millis() - 0x0fff;
    input_buf_size = size;

    input_buf = (char *)malloc(size);
    memset(input_buf, 0x00, size);
    end_sequence(); // init esc_seq
}

ConsoleInput::~ConsoleInput()
{
    free(input_buf);
}

// Get the debug mode
bool ConsoleInput::getDebug()
{
    return flags.debug_mode;
}

// Set the debug mode
void ConsoleInput::setDebug(bool debug)
{
    flags.debug_mode = debug;
}

// Get the debug mode
bool ConsoleInput::getHidePassword()
{
    return flags.hide_password;
}
// Set the debug mode
void ConsoleInput::setHidePassword(bool hide)
{
    flags.hide_password = hide;
}

// Get the debug mode
bool ConsoleInput::getAutoUpdate()
{
    return flags.auto_update;
}

// Set the debug mode
void ConsoleInput::setAutoUpdate(bool update)
{
    flags.auto_update = update;
}

const char *ConsoleInput::getPrompt()
{
    return _prompt;
}
void ConsoleInput::setPrompt(const char *prompt)
{
    if (prompt == NULL)
        return;
    strncpy(_prompt, prompt, sizeof(_prompt) - 1);
    _prompt_len = strlen(_prompt);
}

// ======== Escape Seqences =========================

inline int16_t ConsoleInput::add_sequence()
{
    char key;

    // no input available
    if (stream == NULL || !stream->available())
    {
        last_read = millis();
        return 0;
    }

    // buffer position not available, read but don't buffer
    if (esc_sequence[sizeof(esc_sequence) - 1] != 0)
        return stream->read();

    // it's zero terminated
    size_t pos = strnlen(esc_sequence, sizeof(esc_sequence));
    key = stream->read();
    esc_sequence[pos] = key;

    return key;
}

inline void ConsoleInput::end_sequence()
{
    //  Clear escape sequence buffer
    memset(esc_sequence, 0, sizeof(esc_sequence));
}

inline void ConsoleInput::print_sequence()
{
    if (stream == NULL)
        return;

    // stream->println();
    stream->printf_P(PSTR(" \\e%s => "), esc_sequence + 1);
    for (int i = 1; i < (int)sizeof(esc_sequence); i++)
    {
        if (esc_sequence[i] == 0)
        {
            stream->println();
            if (flags.auto_update)
                update(__LINE__);
            return;
        }
        stream->printf_P(PSTR("0x%2X %c "), esc_sequence[i], esc_sequence[i]);
    }

    stream->println();
    if (flags.auto_update)
        update(__LINE__);
}

// ======== Default Print Methods =========================
int ConsoleInput::available(void)
{
    if (stream == NULL)
        return 0;
    else
        return stream->available();
}

int ConsoleInput::peek(void)
{
    if (stream == NULL)
        return -1;
    else
        return stream->peek();
}

int ConsoleInput::read(void)
{
    last_read = millis();
    if (stream == NULL)
        return 0;
    else
        return stream->read();
}

void ConsoleInput::flush(void)
{
    if (stream)
        stream->flush();
}

size_t ConsoleInput::write(uint8_t c)
{
    if (stream)
        return stream->write(c);
    return 0;
}

// ======== Editing Character Buffer =========================
void ConsoleInput::do_backspace()
{
    if (input_buf == NULL)
        return;

    history_index = 0;

    // history scrolling can make it go out-of-bounds
    size_t len = strnlen(input_buf, input_buf_size);
    if (caret_pos > len)
        caret_pos = len;

    if (caret_pos <= 0)
        return;
    caret_pos--;

    char *src = input_buf + caret_pos + 1;
    char *dst = input_buf + caret_pos;
    memmove(dst, src, len - caret_pos);

    if (flags.auto_update)
        update(__LINE__);
}

void ConsoleInput::do_delete()
{
    if (input_buf == NULL)
        return;

    history_index = 0;

    size_t len = strnlen(input_buf, input_buf_size);
    char *dst = input_buf + caret_pos;
    char *src = input_buf + caret_pos + 1;
    memmove(dst, src, len - caret_pos);

    if (flags.auto_update)
        update(__LINE__);
}

bool ConsoleInput::insertCharacter(char ch, size_t pos)
{
    if (input_buf == NULL)
        return false;

    history_index = 0;

    size_t len = strnlen(input_buf, input_buf_size);

    // history invoke can make the index go out-of-bounds
    if (pos > len)
        pos = len;

    if (pos == len && pos < input_buf_size - 2)
    {
        // expand 1 character to the right
        if (input_buf[pos + 1] != 0)
        {
            // shift right needed
            char *dst = input_buf + len + 1;
            char *src = input_buf + len;
            memmove(dst, src, input_buf_size - len - 1);
        }
        else
        {
            // we still have room
        }
    }

    // Insert character if we have room
    if (pos < input_buf_size - 2)
    {
        if (pos + 1 >= len)
            input_buf[pos + 1] = 0;
        input_buf[pos] = ch;
        pos++;
        return true;
    }

    // Buffer is full
    return false;
}

bool ConsoleInput::insertCharacter(char ch)
{
    if (insertCharacter(ch, caret_pos))
    {
        caret_pos++;
        if (flags.auto_update)
            update(__LINE__);
        return true;
    }

    return false;
}

// ======== Caret Movement =========================

// Get the position of the caret on the input buffer
int16_t ConsoleInput::getCaret()
{
    return caret_pos;
}

// Set the position of the caret on the input buffer
void ConsoleInput::setCaret(int16_t index)
{
    if (input_buf == NULL)
        return;

    history_index = 0;
    size_t len = strnlen(input_buf, input_buf_size);

    if (index > (int16_t)len)
    {
        caret_pos = len;
    }
    else if (index < 0)
    {
        caret_pos = 0;
    }
    else
    {
        caret_pos = index;
    }

    if (flags.auto_update)
        update(__LINE__);
}

// ======== History =========================

size_t ConsoleInput::debugHistorycount()
{
    if (input_buf == NULL)
        return 0;

    size_t count = 0;
    for (size_t i = 1; i < input_buf_size; i++)
    {
        if (input_buf[i] == 0 && input_buf[i - 1] != 0)
            count++;
    }
    return count;
}

size_t ConsoleInput::debugHistoryIndex(size_t num)
{
    size_t pos = 0;
    while (num > 0 && pos < input_buf_size - 2)
    {
        if (input_buf[pos] == 0)
        {
            num--;
            // skip extra \0s
            while (input_buf[pos] == 0)
            {
                pos++;
            }
        }
        else
        {
            pos++;
        }
    }

    return pos;
}

void ConsoleInput::debugShowHistory()
{
    if (stream == NULL)
        return;

    size_t num = debugHistorycount();
    stream->println();
    for (size_t i = 0; i <= num; i++)
    {
        stream->print("[");
        stream->print(i);
        stream->print("] ");
        size_t pos = debugHistoryIndex(i);
        if (pos < input_buf_size)
            stream->println((char *)(input_buf + pos));
    }
}

// ======== Input Line =========================

// Print current input buffer
void ConsoleInput::update(size_t line)
{
    if (stream == NULL)
        return;

    stream->print(F(TERM_CLEAR_LINE)); // Move all the way left + Clear the line
    stream->print(_prompt);

    if (input_buf == NULL)
        return;

    if (flags.debug_mode)
    {
        stream->print(line);

        //        for(uint i = 0; i < input_buf_size; i++) {
        for (uint i = 0; i < 32; i++)
        {
            if (input_buf[i] == 0)
            {
                stream->print("|");
            }
            else
            {
                stream->printf("0x%2x ", input_buf[i]);
                // stream->print((char)input_buf[i]);
            }
        }
        stream->print(line);
        // stream->print(history_index);
        // stream->print("/");
        /*stream->print(debugHistorycount());*/
    }
    else if (!flags.hide_password)
    {
        stream->print(input_buf);
    }

    stream->print("\e[1000D"); // Move all the way left again

    /*if(caret_pos > 0)*/ {
        stream->print("\e[");
        stream->print(flags.hide_password ? _prompt_len : _prompt_len + caret_pos); // Move caret to index
        stream->print("C");
    }
}

void ConsoleInput::setLineCallback(void (*callback)(const char *))
{
    if (flags.auto_update && ((uint16_t)millis() - last_read) >= 0x0fff)
    {
        update(__LINE__);
    }

    line_cb = callback;
}

const char *ConsoleInput::getLine()
{
    return input_buf;
}

void ConsoleInput::pushLine()
{
}

void ConsoleInput::clearLine()
{
    if (input_buf == NULL)
        return;

    size_t len = strnlen(input_buf, input_buf_size);
    memset(input_buf, 0, len);
    setCaret(0);
}

int16_t ConsoleInput::getChar(uint8_t index)
{
    int16_t key;

    // flush buffer if sequence is not closed in timely fashion
    if (esc_sequence[index] != 0x00 && ((uint16_t)millis() - last_read) > 250)
    {
        key = esc_sequence[index];
        memmove(esc_sequence, esc_sequence + 1, sizeof(esc_sequence) - 1);
        esc_sequence[sizeof(esc_sequence) - 1] = 0;
        return key; // flush one char at the time
    }

    last_read = millis();

    // no input available
    if (stream == NULL || !stream->available())
    {
        return 0;
    }

    // buffer position not available
    if (index >= sizeof(esc_sequence))
        return stream->read();

    // buffer position available and used
    if (esc_sequence[index] != 0x00)
        return esc_sequence[index];

    // buffer position available but not used
    key = stream->read();
    esc_sequence[index] = key;

    return key;
}

// Read a key from the terminal or -1 if no key pressed
int16_t ConsoleInput::readKey()
{
    // forced update during constructor, setting last_read to 0x0fff
    if (flags.auto_update && ((uint16_t)millis() - last_read) >= 0x0fff)
    {
        update(__LINE__);
    }

    int16_t key;

    key = getChar(0); // get 1st character in sequence buffer
    if (key <= 0)
        return 0;

    if (key == 0x1b)
    { /* start escape sequence */

        key = getChar(1); // get 2nd character in sequence buffer
        if (key <= 0)
            return 0;

        switch (key)
        {
        case 0x1b: // ESC
            end_sequence();
            return KEY_ESC;

        case '[':
        { // CSI mode
            char appended_key = add_sequence();
            if (appended_key <= 0)
                return 0;

            switch (appended_key)
            {

            case 0x30 ... 0x3F:      // parameter bytes
            case 0x20 ... 0x2F:      // intermediate bytes
                return KEY_BUFFERED; // added to esc_seq, more data in flight

            case 'A':
                // size_t count = debugHistorycount();
                //     if(history_index < count) {
                //         history_index++;
                //         debugGetHistoryLine(history_index);
                //     }
                end_sequence();
                return KEY_UP;

            case 'B':
                // if(history_index > 0) {
                //         history_index--;
                //         debugGetHistoryLine(history_index);
                //     }
                end_sequence();
                return KEY_DOWN;

            case 'C':
                if (flags.auto_move)
                    setCaret(caret_pos + 1);
                end_sequence();
                return KEY_RIGHT;

            case 'D':
                if (flags.auto_move)
                    setCaret(caret_pos - 1);
                end_sequence();
                return KEY_LEFT;

            case 'R':
                // cursor_position returned from query "\e[6n"
                // update_cursor_position();
                end_sequence();
                return KEY_NONE;

            case 'E' ... 'Q':  // End Characters, esc_sequence is complete
            case 'S' ... 0x7D: // End Characters, esc_sequence is complete
            case '@':
                print_sequence();
                end_sequence();
                return KEY_UNKNOWN;

            case '~':
            {
                char *seq = esc_sequence + 2;
                uint8_t size = sizeof(esc_sequence) - 2;
                if (!strncmp_P(seq, PSTR("1~"), size))
                {
                    if (input_buf != NULL && flags.auto_move)
                    {
                        setCaret(0);
                    }
                    end_sequence();
                    return KEY_HOME;
                }
                else if (!strncmp_P(seq, PSTR("2~"), size))
                {
                    end_sequence();
                    return KEY_INSERT;
                }
                else if (!strncmp_P(seq, PSTR("3~"), size))
                {
                    do_delete();
                    end_sequence();
                    return KEY_DELETE;
                }
                else if (!strncmp_P(seq, PSTR("4~"), size))
                {
                    if (input_buf != NULL && flags.auto_move)
                    {
                        setCaret(strnlen(input_buf, input_buf_size));
                    }
                    end_sequence();
                    return KEY_END;
                }
                else if (!strncmp_P(seq, PSTR("5~"), size))
                {
                    end_sequence();
                    return KEY_PAGE_UP;
                }
                else if (!strncmp_P(seq, PSTR("6~"), size))
                {
                    end_sequence();
                    return KEY_PAGE_DOWN;
                }
                else if (!strncmp_P(seq, PSTR("11~"), size))
                {
                    end_sequence();
                    return KEY_FN + 1;
                }
                else if (!strncmp_P(seq, PSTR("12~"), size))
                {
                    end_sequence();
                    return KEY_FN + 2;
                }
                else if (!strncmp_P(seq, PSTR("13~"), size))
                {
                    end_sequence();
                    return KEY_FN + 3;
                }
                else if (!strncmp_P(seq, PSTR("14~"), size))
                {
                    end_sequence();
                    return KEY_FN + 4;
                }
                else if (!strncmp_P(seq, PSTR("15~"), size))
                {
                    end_sequence();
                    return KEY_FN + 5;
                }
                else if (!strncmp_P(seq, PSTR("17~"), size))
                {
                    end_sequence();
                    return KEY_FN + 6;
                }
                else if (!strncmp_P(seq, PSTR("18~"), size))
                {
                    end_sequence();
                    return KEY_FN + 7;
                }
                else if (!strncmp_P(seq, PSTR("19~"), size))
                {
                    end_sequence();
                    return KEY_FN + 8;
                }
                else if (!strncmp_P(seq, PSTR("20~"), size))
                {
                    end_sequence();
                    return KEY_FN + 9;
                }
                else if (!strncmp_P(seq, PSTR("21~"), size))
                {
                    end_sequence();
                    return KEY_FN + 10;
                }
                else if (!strncmp_P(seq, PSTR("23~"), size))
                {
                    end_sequence();
                    return KEY_FN + 11;
                }
                else if (!strncmp_P(seq, PSTR("24~"), size))
                {
                    end_sequence();
                    return KEY_FN + 12;
                }
                print_sequence();
                end_sequence();
            }

                // end_sequence();
                return KEY_BUFFERED; // added to esc_seq, more data in flight

            default:
                // should not happen
                if (stream != NULL)
                    stream->println(F("WARNING !!!"));
                return KEY_BUFFERED; // added to esc_seq
            }
            break;

        } // CSI mode

        case 'O':
        { // CSO mode

            key = getChar(2);
            if (key < 0)
                return 0;
            // print_sequence();

            switch (key)
            {
            case 'P':
                // stream->println(F("F1"));
                end_sequence();
                return KEY_FN + 1;
            case 'Q':
                // stream->println(F("F2"));
                end_sequence();
                return KEY_FN + 2;
            case 'R':
                // stream->println(F("F3"));
                end_sequence();
                return KEY_FN + 3;
            case 'S':
                // stream->println(F("F4"));
                end_sequence();
                return KEY_FN + 4;
            default:
                if (stream != NULL)
                    stream->println(F("UNKNOWN"));
                return KEY_UNKNOWN;
            }
            break;
        } // CSO mode

        default:
            print_sequence();
            end_sequence();
            return KEY_UNKNOWN;
        }
    }
    else if (key == CONSOLE_IAC)
    { /* start IAC sequence */

        uint16_t cmd = getChar(1); // get 2nd character in sequence buffer
        if (cmd <= 0)
            return KEY_BUFFERED;

        uint16_t param = getChar(2); // get 3rd character in sequence buffer
        if (param <= 0)
            return KEY_BUFFERED;

        switch (cmd)
        {
        case CONSOLE_IAC_SEQ_BEGIN:
            // more parameter bytes untill CONSOLE_IAC_SEQ_END
            break;
        case CONSOLE_IAC_WILL:
        case CONSOLE_IAC_WONT:
            break;
        case CONSOLE_IAC_DO:
        case CONSOLE_IAC_DONT:
            switch (param)
            {
            case CONSOLE_IAC_ECHO:           // reply WONT
            case CONSOLE_IAC_NAWS:           // reply WONT
            case CONSOLE_IAC_EXTENDED_ASCII: // reply WONT
                // write(CONSOLE_IAC);
                // write(CONSOLE_IAC_WONT);
                // write(param);
                break;
            default:;
            }
        }

        end_sequence();  // full 3 character IAC sequence has beenread
        return KEY_NONE; // don't do anything
    }

    end_sequence();
    if (key == 0x7f)
        key = 7; // DEL = BACKSPACE

    if (key >= 0x20 && key <= 0xff)
    { // printable characters
        if (flags.auto_edit)
            insertCharacter(key);
        return key;
    }
    else if (key >= 0x00 && key < 0x20)
    {

        switch (key)
        {                   // Ctrl + CHAR
        case KEY_CTRL('A'): // ^A = goto begin
            if (input_buf != NULL && flags.auto_move)
            {
                setCaret(0);
            }
            history_index = 0;
            break;

        case KEY_CTRL('B'): // ^B = go back a word
            if (input_buf != NULL && flags.auto_move)
            {
                setCaret(0);
            }
            history_index = 0;
            break;

        case KEY_CTRL('C'): // ^C = Break
            history_index = 0;
            break;

        case KEY_CTRL('E'): // ^E = goto end
            if (input_buf != NULL && flags.auto_move)
            {
                setCaret(strnlen(input_buf, input_buf_size));
            }
            history_index = 0;
            break;

        case KEY_CTRL('F'): // ^F = go forward a word
            if (input_buf != NULL && flags.auto_move)
            {
                setCaret(strnlen(input_buf, input_buf_size));
            }
            history_index = 0;
            break;

        case KEY_CTRL('G'): // BELL
        case KEY_TAB:
        case KEY_PAUSE:
            return key;

        case KEY_CTRL('H'): // Backspace
            if (flags.auto_edit)
                do_backspace();
            return KEY_BACKSPACE;

        case KEY_LF ... KEY_CR:
        { // LF, VT, FF, CR
            // handle CR/LF
            if (key == KEY_CR && stream != NULL && stream->peek() == KEY_LF)
                stream->read();

            if (input_buf != NULL)
            {
                // if (input_buf[0] != 0 && line_cb != NULL) // let the application handle or ignore empty lines
                if (line_cb != NULL)
                {
                    line_cb(input_buf);
                }
            }

            // if(flags.auto_history) pushHistoryLine(input_buf);

            if (flags.auto_clear)
                clearLine();

            // size_t numchars = 1;
            // memmove(input_buf + numchars, input_buf,
            //         input_buf_size - numchars); // Shift chars right

            // history_index = 0;
            // debugShowHistory();

            return key;
        }
        }
        return key;
    }

    return KEY_UNKNOWN;
}
