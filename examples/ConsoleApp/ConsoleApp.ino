/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <Arduino.h>
#include "ConsoleInput.h"

#define BLINK_TIME 500
#define BUFFER_SIZE 128

ConsoleInput console(&Serial, BUFFER_SIZE);

void parser(const char *input);
void dowork();

void setup()
{
    /* prepare for possible serial debug */
    Serial.begin(115200);
    delay(10);
    pinMode(BUILTIN_LED, OUTPUT);

    console.print(F("\x05"));     // Request Terminal ID
    console.print(F("\x1b\x63")); // Clear Terminal
    /* entered lines are handled by our function */
    console.setLineCallback(parser);

    console.println("ConsoleInput Setup Complete");
    console.println("---------------------------\n");
}

void loop()
{
    /* You can do other stuff here */
    dowork();

    /* Check for a keypress and update the command line */
    uint16_t key = console.readKey(); // required in loop()

    /* Optionally Handle some special command line keys here, if required */
    if (key == ConsoleInput::KEY_PAGE_UP || key == ConsoleInput::KEY_PAGE_DOWN ||
        key == ConsoleInput::KEY_HOME || key == ConsoleInput::KEY_END)
    {
        switch (key)
        {
            console.print(TERM_CLEAR_LINE);

        case ConsoleInput::KEY_PAGE_UP:
            console.println("PAGE_UP pressed");
            break;

        case ConsoleInput::KEY_PAGE_DOWN:
            console.println("PAGE_DOWN pressed");
            break;

        case ConsoleInput::KEY_HOME:
            console.println(TERM_CLEAR_LINE "HOME pressed");
            break;

        case ConsoleInput::KEY_END:
            console.println(TERM_CLEAR_LINE "END pressed");
            break;
        }
        console.update();
    }

    /* The input is already handled via the parser() Callback function
    if (key == ConsoleInput::KEY_LF)
    {
        console.print("You typed :");
        console.println(console.getLine());
        console.clearLine();
    } */

    delay(2);
}

void parser(const char *input)
{
    if (strlen(input) > 0)
    {
        console.println();
        console.print("Hello ");
        console.println(input);
    }
    console.println();
}

bool led_state()
{
    return (millis() / BLINK_TIME) % 2;
}

void dowork()
{
    digitalWrite(BUILTIN_LED, led_state());
}