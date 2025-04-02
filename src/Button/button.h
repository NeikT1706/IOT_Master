#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_


#define NORMAL_STATE 1
#define PRESSED_STATE 0
#define GPIO_PIN_(i) ((uint16_t)(1 << (i)))


void subKeyProcess(int index);
void getKeyInput(int index, int readPin);
int isButtonPressed(int index);

#endif /* INC_BUTTON_H_ */