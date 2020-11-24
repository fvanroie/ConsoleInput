/* MIT License - Copyright (c) 2020 Francis Van Roie francis@netwize.be
   For full license information read the LICENSE file in the project folder */

#ifndef _CONSOLEINPUT_H
#define _CONSOLEINPUT_H

#include <Arduino.h>

#define TERM_CLEAR_LINE "\e[1000D\e[0K"

class ConsoleInput : public Stream
{

private:
  Stream *stream;

  char esc_sequence[10]; // escape sequence buffer
  char *input_buf;       // input buffer and with history
  size_t input_buf_size;
  size_t caret_pos;
  size_t history_index;
  uint16_t last_read;

  struct
  {
    bool insert_mode : 1;
    bool debug_mode : 1;
    bool enable_history : 1;
    bool auto_update : 1;
    bool auto_edit : 1;
    bool auto_move : 1;
    bool auto_clear : 1;
    bool auto_history : 1;
  } flags;

  void end_sequence(void);
  void print_sequence(void);
  int16_t add_sequence();

  void (*line_cb)(const char *);

  void do_backspace();
  void do_delete();

public:
  // Declaration, initialization.
  static const int KEY_NONE = -1;
  static const int KEY_UNKNOWN = 401;

  static const int KEY_LF = 0x0a;
  static const int KEY_CR = 0x0d;
  static const int KEY_PAUSE = 0x1a;
  static const int KEY_ESC = 0x1b;
  static const int KEY_BACKSPACE = 0x08;

  static const int KEY_UP = 256;
  static const int KEY_DOWN = 257;
  static const int KEY_LEFT = 258;
  static const int KEY_RIGHT = 259;

  static const int KEY_PAGE_UP = 260;
  static const int KEY_PAGE_DOWN = 261;
  static const int KEY_INSERT = 262;
  static const int KEY_DELETE = 263;
  static const int KEY_HOME = 264;
  static const int KEY_END = 265;

  static const int MOD_SHIFT = 1 << 10;
  static const int MOD_CTRL = 1 << 11;
  static const int MOD_CMND = 1 << 12;
  static const int MOD_ALT = 1 << 13;
  static const int MOD_ALT_GR = 1 << 14;
  static const int KEY_FN = -512;

  ConsoleInput(Stream *serial, size_t size = 0);
  virtual ~ConsoleInput();

  int16_t readKey();
  int16_t getChar(uint8_t index);

  bool insertCharacter(char ch);
  bool insertCharacter(char ch, size_t pos);

  void setCaret(int16_t index);
  int16_t getCaret(void);
  void update(void);

  void setLineCallback(void (*callback)(const char *));
  const char *getLine();
  void pushLine();
  void clearLine();

  size_t debugHistorycount();
  size_t debugHistoryIndex(size_t num);
  void debugShowHistory();

  virtual int available(void);
  virtual int peek(void);
  virtual int read(void);
  virtual void flush(void);
  virtual size_t write(uint8_t);

  using Print::write;
};

#endif
