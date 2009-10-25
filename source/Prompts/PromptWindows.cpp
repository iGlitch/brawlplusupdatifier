#include <unistd.h>
#include "libwiigui/gui.h"
#include "Prompts/PromptWindows.h"
#include "sys.h"
#include "gecko.h"

/*** Extern variables ***/
extern GuiWindow * mainWindow;
extern GuiSound * bgMusic;
extern u8 shutdown;
extern u8 reset;

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();


/****************************************************************************
* WindowPrompt
*
* Displays a prompt window to user, with information, an error message, or
* presenting a user with a choice of up to 4 Buttons.
*
* Give him 1 Title, 1 Subtitle and 4 Buttons
* If title/subtitle or one of the buttons is not needed give him a 0 on that
* place.
***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label,
const char *btn2Label, const char *btn3Label,
const char *btn4Label)
{
	gprintf("\nWindowPrompt(%s, %s, %s, %s, %s,%s)",title,msg,btn1Label,btn2Label, btn3Label,btn4Label);
    int choice = -1;

    GuiWindow promptWindow(472,320);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);
    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);


    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, 0);
    GuiTrigger trigB;
    trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, 0);

    GuiTrigger trigGCA;
	trigGCA.SetButtonOnlyTrigger(-1, 0 | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigGCB;
	trigGCB.SetButtonOnlyTrigger(-1, 0 | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	GuiTrigger trigGCX;
	trigGCX.SetButtonOnlyTrigger(-1, 0 | WPAD_CLASSIC_BUTTON_X, PAD_BUTTON_X);
	GuiTrigger trigGCY;
	trigGCY.SetButtonOnlyTrigger(-1, 0 | WPAD_CLASSIC_BUTTON_Y, PAD_BUTTON_Y);
	GuiImageData dialogBox(dialogue_box_png);
    GuiImage dialogBoxImg(&dialogBox);

    GuiText titleTxt(title, 26, (GXColor){0, 0, 0, 255});
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0,55);
    titleTxt.SetMaxWidth(430, GuiText::SCROLL);

    GuiText msgTxt(msg, 22, (GXColor){0, 0, 0, 255});
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0,-40);
    msgTxt.SetMaxWidth(430, GuiText::DOTTED);

    GuiText btn1Txt(btn1Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn1Img(&btnOutline);
    GuiImage btn1ImgOver(&btnOutlineOver);
    GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn1.SetLabel(&btn1Txt);
    btn1.SetImage(&btn1Img);
    btn1.SetImageOver(&btn1ImgOver);
    btn1.SetSoundOver(&btnSoundOver);
    btn1.SetSoundClick(&btnClick);
    btn1.SetTrigger(&trigA);
    btn1.SetTrigger(&trigGCB);
    btn1.SetState(STATE_SELECTED);
    btn1.SetEffectGrow();


    GuiText btn2Txt(btn2Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn2Img(&btnOutline);
	GuiImage btn2ImgOver(&btnOutlineOver);
    GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn2.SetLabel(&btn2Txt);
    btn2.SetImageOver(&btn2ImgOver);
    btn2.SetImage(&btn2Img);
    btn2.SetSoundOver(&btnSoundOver);
    btn2.SetSoundClick(&btnClick);
    if(!btn3Label && !btn4Label)
    btn2.SetTrigger(&trigB);
    btn2.SetTrigger(&trigGCY);
    btn2.SetTrigger(&trigA);
    btn2.SetEffectGrow();

    GuiText btn3Txt(btn3Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn3Img(&btnOutline);
	GuiImage btn3ImgOver(&btnOutlineOver);
    GuiButton btn3(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn3.SetLabel(&btn3Txt);
    btn3.SetImage(&btn3Img);
    btn3.SetImageOver(&btn3ImgOver);
    btn3.SetSoundOver(&btnSoundOver);
    btn3.SetSoundClick(&btnClick);
    if(!btn4Label)
    btn3.SetTrigger(&trigB);
    btn3.SetTrigger(&trigA);
    btn3.SetTrigger(&trigGCA);
    btn3.SetEffectGrow();

    GuiText btn4Txt(btn4Label, 22, (GXColor){0, 0, 0, 255});
    GuiImage btn4Img(&btnOutline);
	GuiImage btn4ImgOver(&btnOutlineOver);
    GuiButton btn4(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn4.SetLabel(&btn4Txt);
    btn4.SetImageOver(&btn4ImgOver);
    btn4.SetImage(&btn4Img);
    btn4.SetSoundOver(&btnSoundOver);
    btn4.SetSoundClick(&btnClick);
    if(btn4Label)
    btn4.SetTrigger(&trigB);
    btn4.SetTrigger(&trigA);
    btn4.SetTrigger(&trigGCX);
    btn4.SetEffectGrow();

    if(btn2Label && !btn3Label && !btn4Label) {
    btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn1.SetPosition(40, -45);
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-40, -45);
    btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn3.SetPosition(50, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    } else if(btn2Label && btn3Label && !btn4Label) {
    btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn1.SetPosition(50, -120);
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-50, -120);
    btn3.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    btn3.SetPosition(0, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    } else if(btn2Label && btn3Label && btn4Label) {
    btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn1.SetPosition(50, -120);
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-50, -120);
    btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn3.SetPosition(50, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    } else if(!btn2Label && btn3Label && btn4Label) {
    btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    btn1.SetPosition(0, -120);
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-50, -120);
    btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn3.SetPosition(50, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    } else {
    btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
    btn1.SetPosition(0, -45);
    btn2.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn2.SetPosition(50, -120);
    btn3.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    btn3.SetPosition(50, -65);
    btn4.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn4.SetPosition(-50, -65);
    }

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&msgTxt);

    if(btn1Label)
    promptWindow.Append(&btn1);
    if(btn2Label)
    promptWindow.Append(&btn2);
    if(btn3Label)
    promptWindow.Append(&btn3);
    if(btn4Label)
    promptWindow.Append(&btn4);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while(choice == -1)
    {
        VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();

        if(reset == 1)
            Sys_Reboot();

        if(btn1.GetState() == STATE_CLICKED) {
            choice = 1;
        }
        else if(btn2.GetState() == STATE_CLICKED) {
        if(!btn3Label)
            choice = 0;
        else
            choice = 2;
        }
        else if(btn3.GetState() == STATE_CLICKED) {
        if(!btn4Label)
            choice = 0;
        else
            choice = 3;
        }
        else if(btn4.GetState() == STATE_CLICKED) {
            choice = 0;
        }
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(promptWindow.GetEffect() > 0) usleep(50);
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
	gprintf(" = %i",choice);
    return choice;
}
int OnScreenKeyboard(char * var, u16 maxlen)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText okBtnTxt("OK", 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(&btnSoundOver);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 22, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&keyboard);
	mainWindow->ChangeFocus(&keyboard);
	ResumeGui();

	while(save == -1)
	{
		VIDEO_WaitVSync();

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if(save)
	{
		snprintf(var, maxlen, "%s", keyboard.kbtextstr);
	}

	HaltGui();
	mainWindow->Remove(&keyboard);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return save;
}

