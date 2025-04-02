#include "button.h"
#include <Arduino.h>
int keyReg0[3] = {NORMAL_STATE};
int keyReg1[3] = {NORMAL_STATE};
int keyReg2[3] = {NORMAL_STATE};

int keyReg3[3] = {NORMAL_STATE};
int timerForKeyPress = 200;
int button_flag[4] = {0, 0, 0, 0};

int isButtonPressed(int index)
{
    if (button_flag[index] == 1)
    {
        button_flag[index] = 0;
        return 1;
    }
    return 0;
}

void subKeyProcess(int index)
{
    button_flag[index] = 1;
}

void getKeyInput(int index, int readPin)
{
    keyReg0[index] = keyReg1[index];
    keyReg1[index] = keyReg2[index];

    keyReg2[index] = digitalRead(readPin);
    if ((keyReg0[index] == keyReg1[index]) && (keyReg1[index] == keyReg2[index]))
    {
        if (keyReg3[index] != keyReg2[index])
        {
            keyReg3[index] = keyReg2[index];
            if (keyReg2[index] == PRESSED_STATE)
            {
                subKeyProcess(index);
                timerForKeyPress = 200;
            }
        }
        else
        {
            timerForKeyPress--;
            if (timerForKeyPress == 0)
            {
                keyReg3[index] = NORMAL_STATE;
            }
        }
    }
}