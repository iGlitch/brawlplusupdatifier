/****************************************************************************
 * PromptWindows
 * dimok
 *
 * PromptWindows.h
 ***************************************************************************/

#ifndef _PROMPTWINDOWS_H_
#define _PROMPTWINDOWS_H_
int WindowPrompt(const char *title, const char *msg = NULL, const char *btn1Label = NULL,
                    const char *btn2Label = NULL, const char *btn3Label = NULL,
                    const char *btn4Label = NULL);
int OnScreenKeyboard(char * var, u16 maxlen);
#endif
