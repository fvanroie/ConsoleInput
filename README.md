# ConsoleInput

[![GitHub release](https://img.shields.io/github/release/fvanroie/ConsoleInput.svg)](https://github.com/fvanroie/ConsoleInput/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/fvanroie/ConsoleInput/blob/master/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/fvanroie/ConsoleInput.svg)](http://github.com/fvanroie/ConsoleInput/issues)
[![Discord](https://img.shields.io/discord/538814618106331137?color=%237289DA&label=support&logo=discord&logoColor=white)][1]

With this arduino library you can create an interactive command prompt experience, similar to a shell.
You can then connect to your MCU using a virtual terminal application and type commands.
The prompt is updated on-the-fly and the program is not blocked while waiting for key presses.

Special keys like HOME, END, function keys, ESC... are also recognozed and can be handled in your program if required.

A callback routine can be set that will be called whenever ENTER/RETURN is pressed so you can handle the user input.
Check out the example in the examples folder.

### Include ConsoleInput

```cpp
#include "ConsoleInput.h"
ConsoleInput console(&Serial, BUFFER_SIZE);
```

### Read Key

In the main loop repeatedly call `readKey()` to check for input.
This call also ensures the commandline updates are processed, like text input, arrow keys, Home and End keys...

The `readKey()` function is non-blocking and immediately returns `ConsoleInput::KEY_NONE` if no input is detected.

```cpp
void loop()
{
    /* Non-blocking check for a keypress and update the command line */
    uint16_t key = console.readKey(); // required in loop()
}
```

Even if you don't need the actual `key` value in your program, you must call `readKey()` to ensure the commandline can process the incoming characters.

### Line Callback

Optionally set a callback function that is called whenever `CR/LF`, `CR` or `LF` is detected.

```cpp
void setup()
{
    /* entered lines are handled by the parser function */
    console.setLineCallback(parser);
}
```

### Input Handling

The input callback function must have a signature `void func(const char *)`.
It is called with a pointer to the current text on the commandline.

```cpp
void parser(const char *input)
{
        console.print("Hello ");
        console.println(input);
}
```

### Special Keys

Handling special key input is easy by just checking against the library constants.

```cpp
void loop()
{
    uint16_t key = console.readKey(); // required in loop()

    /* Optionally Handle some special keys */
    if (key == ConsoleInput::KEY_PAGE_UP) {
        console.println("PAGE_UP pressed");
    }
}
```

## [Buy me a coffee](https://www.buymeacoffee.com/aktdCofU)

[![Buy me a coffee](https://www.buymeacoffee.com/assets/img/custom_images/black_img.png)](https://www.buymeacoffee.com/aktdCofU)


[1]: https://discord.gg/VCWyuhF
