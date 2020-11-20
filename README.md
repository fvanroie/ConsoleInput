# ConsoleInput

[![GitHub release](https://img.shields.io/github/release/fvanroie/ConsoleInput.svg)](https://github.com/fvanroie/ConsoleInput/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/fvanroie/ConsoleInput/blob/master/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/fvanroie/ConsoleInput.svg)](http://github.com/fvanroie/ConsoleInput/issues)
[![Discord](https://img.shields.io/discord/538814618106331137?color=%237289DA&label=support&logo=discord&logoColor=white)][1]

With this library you can create an interactive command prompt experience, similar to a shell. You can connect it your MCU and type commands.
The prompt is updated on-the-fly and the program is not blocked waiting for key presses. Special keys like HOME, END, function keys, ESC...
are also recognozed and can be handled in your program is required.

A callback routine can be set which will execute when ENTER/RETURN is pressed, so you can parse and handle the user input.
Check out the example in the examples folder.

### Include ConsoleInput

```cpp
#include "ConsoleInput.h"
ConsoleInput console(&Serial, BUFFER_SIZE);
```

### Readkeys

In the main loop repeadedly call `readKey()` to check for input.
This call also ensures the commandline updates are processed, like text input, arrow keys, Home and End keys...

The `readKey()` function is non-blocking and immediately returns `ConsoleInput::KEY_NONE` if no input is detected.

```cpp
void loop()
{
    /* Non-blocking check for a keypress and update the command line */
    uint16_t key = console.readKey(); // required in loop()
}
```

### Callback

Optionally set a callback function that is called whenever `CR/LF`, `CR` or `LF` is detected.

```cpp
void setup()
{
    /* entered lines are handled by the parser function */
    console.setLineCallback(parser);
}
```

### Input Handling

The input callback function must have a signature `void func(const char *input)`.
It is called with a pointer to the current text on the commandline.

```
void parser(const char *input)
{
        console.print("Hello ");
        console.println(input);
}
```

[1]: https://discord.gg/VCWyuhF
