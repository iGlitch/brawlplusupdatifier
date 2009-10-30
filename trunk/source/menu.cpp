/****************************************************************************
 * menu.cpp
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <iostream>
#include <dirent.h>

#include "libwiigui/gui.h"
#include "menu.h"
#include "main.h"
#include "input.h"
#include "Prompts/PromptWindows.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "sys.h"
#include "fatmounter.h"
#include "wpad.h"
#include "gecko.h"
#include "Boothomebrew.h"
#include "MD5.h"
#include "Networkops.h"
#include "http.h"
#include "Boothomebrew.h"
#include "gecko.h"

#define textcolor		(GXColor){77, 51, 25, 255}

//#define infotextcolor		(GXColor){255,255,0, 255}
#define CONSOLELEFT		20
#define NUMLINES		12
#define VERSIONTEXT		"Brawl+ Updatifier 1.1 by giantpune"

extern bool networkinitialized;
extern char output[];
static int fctn=0,fctn2=0;
char out[400];//probably don't need 400 for some of these, but just being safe
static char	path[400], //also probably don't need all of them to be global
			url[400], 
			url2[400], 
			hash[150], 
			temp1[200], 
			temp2[200],
			t[400];
unsigned char hash2[150];
static u8 renam=0;
static bool play=false;

static GuiImage *progress1fullImg=NULL;
static GuiButton *ExitBtn=NULL;
GuiWindow * mainWindow = NULL;
static GuiWindow * w = NULL;
static GuiText ** infoTxt = NULL;
static GuiSound * bgMusic = NULL;
static GuiImageData * pointer[4];
static GuiImage * bgImg = NULL;
static GuiImage * bgConsoleImg = NULL;


static char linebuf[NUMLINES][200];
static s8 curline = 0;
static lwp_t guithread = LWP_THREAD_NULL;
static bool guiHalt = true;
static int ExitRequested = 0;


extern u8 shutdown;
extern u8 reset;

extern lwp_t brawlthread;
extern int ExitBrRequested;

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
void ResumeGui()
{
	guiHalt = false;
	LWP_ResumeThread (guithread);
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
void HaltGui()
{
	guiHalt = true;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(guithread))
		usleep(THREAD_SLEEP);
}

/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/

static void *
UpdateGUI (void *arg)
{
	int i;

	while(1)
	{
		if(guiHalt)
		{
			LWP_SuspendThread(guithread);
		}
		else
		{
			mainWindow->Draw();

			#ifdef HW_RVL
			for(i=3; i >= 0; i--) // so that player 1's cursor appears on top!
			{
				if(userInput[i].wpad.ir.valid)
					Menu_DrawImg(userInput[i].wpad.ir.x-48, userInput[i].wpad.ir.y-48,
						96, 96, pointer[i]->GetImage(), userInput[i].wpad.ir.angle, 1, 1, 255);
			}
			#endif

			Menu_Render();

			for(i=0; i < 4; i++)
				mainWindow->Update(&userInput[i]);

			if(ExitRequested)
			{
				for(i = 0; i < 256; i += 15)
				{
					mainWindow->Draw();
					Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor){0, 0, 0, i},1);
					Menu_Render();
				}
                ExitRequested = 2;
                return NULL;
			}
		}
	}
	return NULL;
}

/****************************************************************************
 * InitThread
 *
 * Startup threads
 ***************************************************************************/
void InitThreads()
{
    //!Initialize main GUI handling thread
	LWP_CreateThread (&guithread, UpdateGUI, NULL, NULL, 0, 90);
	
}

/****************************************************************************
 * ExitGUIThread
 *
 * Shutdown GUI threads
 ***************************************************************************/
void ExitGUIThreads()
{
	ExitRequested = 1;
	LWP_JoinThread(guithread, NULL);
	guithread = LWP_THREAD_NULL;
}

static bool pageFilled =false;
void updateList(const char *in, bool skip)
{
	HaltGui();
	
	
	if (skip)//if there is a new line
	{
		if(!pageFilled)
			{
				snprintf(linebuf[curline],sizeof(linebuf[curline]),"%s",in);
				infoTxt[curline]->SetText(linebuf[curline]);
				
				
				if (curline==NUMLINES-1)
				{
					pageFilled=true;
				}	
				
			}
		else {
		
			for (int i=0;i<NUMLINES;i++)
				{
					infoTxt[i]->SetText(linebuf[(i+curline+1)%NUMLINES]);
				}
			
			snprintf(linebuf[curline],sizeof(linebuf[curline]),"%s",in);
			infoTxt[NUMLINES-1]->SetText(linebuf[curline]);
			
			
		
		}
		curline=((curline+1)%(NUMLINES));
	}
	else {
	
		snprintf(linebuf[curline],sizeof(linebuf[curline]),"%s%s",linebuf[curline],in);
		infoTxt[curline]->SetText(linebuf[curline]);
	}
	ResumeGui();
}

void clearConsole()
{

	for (int i=0;i<NUMLINES;i++)
	{
		
		infoTxt[i]->SetText(" ");
		sprintf(linebuf[i]," ");
	}
	pageFilled =false;
	curline =0;
}

void ex(const char *l)
{
	//mainWindow->SetBlackbox(false,CONSOLELEFT,0,screenwidth-CONSOLELEFT,500,(GXColor){0,0,0,100});
	mainWindow->Remove(bgConsoleImg);
	WindowPrompt("Error!",l,"OK");
	ExitApp();
	gprintf("\nExiting... %s",l);
	while(ExitRequested != 2) usleep(THREAD_SLEEP);

	bgMusic->Stop();
	delete bgMusic;
	delete bgImg;
	delete bgConsoleImg;
	delete mainWindow;
	delete pointer[0];
	delete pointer[1];
	delete pointer[2];
	delete pointer[3];
	
	delete progress1fullImg;
	delete ExitBtn;
	delete w;
	for (int i=0;i<NUMLINES;i++)
	{
		delete(infoTxt[i]);	
	}
	infoTxt = NULL;
	exit(0);
}
static bool CheckFile(const char * filepath)
{
    FILE * f;
    f = fopen(filepath,"rb");
    if(f) {
    fclose(f);
    return true;
    }
    fclose(f);
    return false;
}

static bool brawlDL(const char *url, const char *path)
{
	char tmp[300];
	int ret = 0, failed = 0;
	bool re =false;
	sprintf(tmp,"sd:/tmp.tmp");
	s32 filesize = download_request(url);
	//gprintf("\nexpected filesize=%i",filesize);
    if (filesize > 0) {
        FILE * pfile;
        pfile = fopen(tmp, "wb");
        u8 * blockbuffer = new unsigned char[NETWORKBLOCKSIZE];
        for (s32 i = 0; i < filesize; i += NETWORKBLOCKSIZE) {
            usleep(100);
			
			HaltGui();
			progress1fullImg->SetTile(40*i/filesize);
			ResumeGui();

            if(ExitBtn->GetState() == STATE_CLICKED) {
                fclose(pfile);
                remove(tmp);
                failed = -1;
				gprintf("...Exit clicked");
                break;
            }

            u32 blksize;
            blksize = (u32)(filesize - i);
            if (blksize > NETWORKBLOCKSIZE)
                blksize = NETWORKBLOCKSIZE;

            ret = network_read(blockbuffer, blksize);
            if (ret != (s32) blksize) {
                failed = -1;
                ret = -1;
                fclose(pfile);
                remove(tmp);
				re = false;
				//gprintf("...block expected =%i, received =%i",blksize,ret);
                break;
            }
            fwrite(blockbuffer,1,blksize, pfile);
			//gprintf("...block %i written",i);
        }
		fclose(pfile);
		CloseConnection();
		delete blockbuffer;
		//gprintf("... delete blockbuffer;");
        if (!failed) {
                        //remove old
			if (CheckFile(path)) {
				remove(path);
			}
			rename(tmp, path);
			//gprintf("...Saved");
			re=true;
		}


	}
	return re;

}

static char *getFoldername(const char *in)
{
	char * pch = NULL;
	strlcpy(temp1, in, sizeof(temp1));
	pch = strrchr(temp1,'/'); 
	if (pch != NULL)
		*pch = 0;
		
	sprintf(temp1,"%s%c%c",temp1,'/','\0');
	
	//gprintf("\ninput = %s:  output = \"%s\"",in,temp1);
	return temp1;
}

static char *shortname(const char *in)
{
	int len = strlen(in), l=0;
	memset(temp2,0,sizeof(temp2));
	
	for(int i=len;i>=0 && in[i]!='/';i--)l=i;
		
	for(int i=0;i<(len-l);i++)
		temp2[i]=in[i+l];
		
		
	//gprintf("\ninput = %s:  output = \"%s\"",in,temp2);
	return temp2;
} 

bool checkfilehash(const char * p, const char * h, bool recheck = false)
{
//gprintf("\ncheckfilehash(%s,\"%s\",%i)",p, h,recheck);
	FILE *f = fopen (p ,"rb");
	if(!f)
	{
		gprintf("\n%s does not exist",p);
		
		snprintf(t,sizeof(t),"%s does not exist",p);
		updateList(t,true);
		
		fclose(f);
		return false;
	}//gprintf("\nopen file");
	
	

	bool ret = false;
	fseek (f, 0, SEEK_END);
    long size=ftell (f);
    rewind (f);
	//gprintf("\nsize of %s is %i",p,s);
	char *tmp = (char *) malloc(size);
	if (!tmp)ex("out of memory");
	fread(tmp,1,size,f);
	rewind (f);
	fclose(f);
	//gprintf("\nclosed file size = %i",s);
	
	MD5 hash;//gprintf("MD5 started");
	hash.update(tmp,size);
	hash.finalize();
	if(!recheck)
	{
		gprintf("\n%s",p);
		gprintf("\ncurrent hash %s",hash.hexdigest().c_str());
		gprintf("\nneeded hash  %s",h);
		
		snprintf(t,sizeof(t),"%s",p);
		updateList(t,true);
		snprintf(t,sizeof(t),"current hash    %s",hash.hexdigest().c_str());
		updateList(t,true);
		snprintf(t,sizeof(t),"needed hash  %s",h);
		updateList(t,true);
	}
	
	if (strcmp(h, hash.hexdigest().c_str()) == 0)
	{
		if(!recheck)
		{
			gprintf("\nfile already exists on the SD");
			updateList("file already exists on the SD",true);
		}
		progress1fullImg->SetTile(40);
		ret = true;
	}
	else 
	{
		if (!(recheck) && renam!=2)
		{
			HaltGui();
			mainWindow->Remove(w);
			for (int i=0;i<NUMLINES;i++)
			{
				w->Remove(infoTxt[i]);	
			}
			bgImg->SetAlpha(100);
			bgConsoleImg->SetAlpha(0);
			//mainWindow->Remove(bgConsoleImg);
			//mainWindow->SetBlackbox(false,CONSOLELEFT,0,screenwidth-CONSOLELEFT,500,(GXColor){0,0,0,100});
			
			ResumeGui();
			//1,2,0
			renam = WindowPrompt(path,"Do you want to overwrite your current file?","Yes","Overwrite all","Rename Current");
			if (renam==0)
			{
				char entered[39];
				sprintf(entered,"%s",shortname(p));
                int result = OnScreenKeyboard(entered, 40);
                if ( result == 1 ) {
					char tmp1[200], tmp2[200];
                    sprintf(tmp1,"%s%s",getFoldername(p),shortname(p));
					sprintf(tmp2,"%s%s",getFoldername(p),entered);
					//gprintf("\n %s to %s",tmp1,tmp2);
					if (CheckFile(tmp2)) {
						//gprintf("\ncheckfile(%s) = yes",tmp2);
							remove(tmp2);
					}
					rename(tmp1, tmp2);
					gprintf("\nrenamed %s to %s",tmp1,tmp2);
					snprintf(t,sizeof(t),"renamed %s to %s",tmp1,tmp2);
					updateList(t,true);
				} 
			}
			HaltGui();
			mainWindow->Append(bgConsoleImg);
			mainWindow->Append(w);
			for (int i=0;i<NUMLINES;i++)
			{
				w->Append(infoTxt[i]);	
			}
			bgImg->SetAlpha(255);
			bgConsoleImg->SetAlpha(170);
			//mainWindow->SetBlackbox(true,CONSOLELEFT,0,screenwidth-CONSOLELEFT,500,(GXColor){0,0,0,100});
			ResumeGui();
		}
	}
	
	free(tmp);
	//gprintf("\nhash check done");
	return ret;

}






static int parseLine1(const char *in)
{
	u16 i,z=0,j,k;
	if (in[0]=='+')return 0;
	if (in[0]=='-'){gprintf(" Abort! ");return -1;}
	
//gprintf("\nparseLine(%s,%s,%s,%s)\nhash =",in,url,path,hash);	
	memset(path,0,sizeof(path));
	memset(url,0,sizeof(url));
	memset(hash,0,sizeof(hash));
	for (i=0;i<strlen(in);i++)
	{
		if(in[i]!=':')
			url[i]=in[i];
		else 
		{
			z=i;
			url[i]='\0';
			break;
		}
	}
	z++;
	for (j=0;j<strlen(in);j++)
	{
		if(in[j+z]!=':')
			path[j]=in[j+z];
		else 
		{
			z=j+z;
			url[i]='\0';
			break;
		}
	}
	z++;
	for (k=0;k<strlen(in)-1;k++)
	{
		if(in[k+z]!='\0'&&in[k+z]!='\n'&&in[k+z]!=0x0d)
			{
			hash[k]=in[k+z];
			//gprintf("%02x",hash[k]);
		
		}else 
		{
			z=k+z;
			url[i]='\0';
			break;
		}
	}
	if (!(strlen(path)>0&&strlen(hash)>0&&strlen(url)>0))
	{	
		gprintf("\tparseline failed");
		return 0;
	}

	return 1;
}

static bool subfoldercreate(const char * fullpath) {
	//check forsubfolders
	char dir[300];
	char * pch = NULL;
	u32 len;
	struct stat st;

	strlcpy(dir, fullpath, sizeof(dir));
	len = strlen(dir);
	if(len && len< sizeof(dir)-2 && dir[len-1] != '/');
	{
		dir[len++] = '/';
		dir[len] = '\0';
	}
	if (stat(dir, &st) != 0) // fullpath not exist?
	{
		while(len && dir[len-1] == '/')
			dir[--len] = '\0';				// remove all trailing /
		pch = strrchr(dir, '/');
		if(pch == NULL) return false;
		*pch = '\0';
		if(subfoldercreate(dir))
		{
			*pch = '/';
			if (mkdir(dir, 0777) == -1)
				return false;
		}
		else
			return false;
	}
	return true;
}


static bool makefolder(const char *in)
{
	char tmp[400];
	memset(tmp,0,sizeof(tmp));
	
	u16 i,j=0;
	for(i=strlen(in);i>1;i--)
	{
		if (in[i]=='/')
		{
			j=i;
			break;
		}
	}
	for(i=0;i<j+1;i++)
		tmp[i]=in[i];
//	gprintf("\nfolder = \"%s\"",tmp);
	
    struct stat st;
    if (stat(tmp, &st) != 0) {
        if (subfoldercreate(tmp) != 1) {
		gprintf("\nfolder not created");
            return false;
        }
    }
	return true;


}




bool cfg_save(bool m, bool usb) {
	bool uussbb = usb;
	bool mm = m;
gprintf("\ncfg_save(%d, %d)",mm,uussbb);
    char Global_cfg[26];
    FILE *f;
    sprintf(Global_cfg, "sd:/geckoBootInstructions.cfg");
    f = fopen(Global_cfg, "w");
    if (!f) {
        return false;
    }
    fprintf(f, "# Note: This file is automatically generated\n");
    fclose(f);
    // Closing and reopening because of a write issue we are having right now /
    f = fopen(Global_cfg, "w");
	fprintf(f, "# Note: This file is automatically generated\n");
    fprintf(f, "%s\n",(mm?"IOS222":"IOS36"));
    fprintf(f, "%s\n",(uussbb?"bootfromUSB":" "));
    fclose(f);
	gprintf("\nsettings saved");
	gprintf("\nwrote \"%s\"\n",(mm?"IOS222":"IOS36"));
    gprintf("\nwrote \"%s\"\n",(uussbb?"bootfromUSB":" "));
    
	return true;
}

static int MenuUpdate()
{
	
//	bool btn=false;
	int pro=100, i=30, hashfails=0;
	
	int menu = MENU_NONE;
	
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | 0, 0);
	GuiTrigger trigBO1;
	trigBO1.SetButtonOnlyTrigger(-1, WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_Y, PAD_BUTTON_Y);
	GuiTrigger trigBO2;
	trigBO2.SetButtonOnlyTrigger(-1, WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_X, PAD_BUTTON_X);
	
	GuiTrigger trigH;
	trigH.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);
	
	GuiSound btnClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    
/*	////////////for testing
	GuiTrigger trigU;
	trigU.SetButtonOnlyTrigger(-1, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP, PAD_BUTTON_START);
	
	GuiTrigger trigD;
	trigD.SetButtonOnlyTrigger(-1, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);
	
	GuiTrigger trigL;
	trigL.SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);
	
	GuiTrigger trigR;
	trigR.SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_START);
	///////////////
	
	
	
	GuiTrigger trigGCA;
	trigGCA.SetButtonOnlyTrigger(-1, 0 | 0, PAD_BUTTON_A);
	GuiTrigger trigGCB;
	trigGCB.SetButtonOnlyTrigger(-1, 0 | 0, PAD_BUTTON_B);
	GuiTrigger trigGCX;
	trigGCX.SetButtonOnlyTrigger(-1, 0 | 0, PAD_BUTTON_X);
	GuiTrigger trigGCY;
	trigGCY.SetButtonOnlyTrigger(-1, 0 | 0, PAD_BUTTON_Y);
*/	
  
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineover(button_over_png);
//	int buttonSpacing = 15;
//	int buttonX = 25, buttonY = 150;
	
	GuiText updateBtnTxt("Update", 24, textcolor);
	GuiImage updateBtnImg(&btnOutline);
	GuiImage updateOverBtnImg(&btnOutlineover);
	GuiButton updateBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	updateBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	updateBtn.SetPosition(0,308);//(buttonX, buttonY);
	updateBtn.SetLabel(&updateBtnTxt);
	updateBtn.SetImage(&updateBtnImg);
	updateBtn.SetImageOver(&updateOverBtnImg);
	updateBtn.SetTrigger(&trigA);
	updateBtn.SetTrigger(&trigBO1);
	updateBtn.SetEffectGrow();
	updateBtn.SetSoundClick(&btnClick);
	updateBtn.SetSoundOver(&btnSoundOver);
	//buttonY += btnOutline.GetHeight()+buttonSpacing;

	GuiText playBtnTxt("Play", 24, textcolor);
	GuiImage playBtnImg(&btnOutline);
	GuiImage playBtnOverImg(&btnOutlineover);
	GuiButton playBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	playBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);//(ALIGN_LEFT, ALIGN_TOP);
	playBtn.SetPosition(0,353);//(buttonX, buttonY);
	playBtn.SetLabel(&playBtnTxt);
	playBtn.SetImage(&playBtnImg);
	playBtn.SetImageOver(&playBtnOverImg);
	playBtn.SetTrigger(&trigA);
	playBtn.SetTrigger(&trigBO2);
	playBtn.SetEffectGrow();
	playBtn.SetSoundClick(&btnClick);
	playBtn.SetSoundOver(&btnSoundOver);
	//buttonY += btnOutline.GetHeight()+buttonSpacing;

	GuiImageData progress1data(progressbar_outline_dark_png);
	GuiImageData progress2data(progressbar_outline_png);
	GuiImageData progress1fulldata(progressbar_png);
	GuiImageData progress2fulldata(progressbar_png);
	GuiImageData progress1emptydata(progressbar_empty_png);
	GuiImageData progress2emptydata(progressbar_empty_png);
	
	
	GuiImage progress1Img(&progress1data);
	GuiImage progress2Img(&progress2data);
	progress1fullImg = new GuiImage(&progress1fulldata);
	GuiImage progress2fullImg(&progress2fulldata);
	GuiImage progress1emptyImg(&progress1emptydata);
	GuiImage progress2emptyImg(&progress2emptydata);
	
	progress1Img.SetAlignment(0,3);
	progress2Img.SetAlignment(0,3);
	progress1Img.SetPosition(21,308);
	progress2Img.SetPosition(22,353);
	
	progress1fullImg->SetAlignment(0,3);
	progress2fullImg.SetAlignment(0,3);
	progress1fullImg->SetPosition(21,308);
	progress2fullImg.SetPosition(22,353);
	progress1fullImg->SetTile(0);
	progress2fullImg.SetTile(0);
	
	progress1emptyImg.SetAlignment(0,3);
	progress2emptyImg.SetAlignment(0,3);
	progress1emptyImg.SetPosition(21,308);
	progress2emptyImg.SetPosition(22,353);
	progress1emptyImg.SetTile(40);
	progress2emptyImg.SetTile(40);
	
	customOptionList *report = NULL;
	GuiCustomOptionBrowser *optionBrowser = NULL;
	
	GuiImageData bgConsoleData(bg_options_settings_png);
    bgConsoleImg = new GuiImage(&bgConsoleData);
	bgConsoleImg->SetPosition(20, 20);
	bgConsoleImg->SetAlpha(0);
	
	infoTxt = new GuiText * [NUMLINES];
	
	int infoX = CONSOLELEFT+20, infoY = 32, infoSpace = 2, infoSize = 20;
	
	for (int i=0;i<NUMLINES;i++)
	{
		sprintf(linebuf[i]," ");
		
		infoTxt[i] = new GuiText(linebuf[i],infoSize,infotextcolor);
		infoTxt[i]->SetAlignment(0,3);
		infoTxt[i]->SetPosition(infoX,infoY);
		infoTxt[i]->SetMaxWidth((screenwidth-infoX)-50,GuiText::DOTTED);
		
		infoY += infoSize+infoSpace;
	}


/*	//for testing	
	GuiButton UBtn(0,0);
	GuiButton DBtn(0,0);
	GuiButton LBtn(0,0);
	GuiButton RBtn(0,0);
	UBtn.SetTrigger(&trigU);
	DBtn.SetTrigger(&trigD);
	LBtn.SetTrigger(&trigL);
	RBtn.SetTrigger(&trigR);
*/	
	ExitBtn = new GuiButton(0,0);
	ExitBtn->SetTrigger(&trigH);
	
	HaltGui();
	w = new GuiWindow(screenwidth, screenheight);
	w->Append(&playBtn);
	w->Append(&updateBtn);
	//blablabla...testing..blablabla
	/*w->Append(&UBtn);
	w->Append(&DBtn);
	w->Append(&RBtn);
	w->Append(&LBtn);*/
	w->Append(ExitBtn);
	//w->Append(&progress1emptyImg);
	//w->Append(&progress2emptyImg);
	//w->Append(progress1fullImg);
	//w->Append(&progress2fullImg);
	//w->Append(&progress1Img);
	//w->Append(&progress2Img);
	mainWindow->Append(bgConsoleImg);
	mainWindow->Append(w);
	ResumeGui();
//	bool changed = true;
	int numFiles=0;
	while(menu == MENU_NONE)
	{
	
	
	
		VIDEO_WaitVSync();

        if(shutdown == 1)
            Sys_Shutdown();

        else if(reset == 1)
            Sys_Reboot();

		else if(ExitBtn->GetState() == STATE_CLICKED)
			menu = MENU_EXIT;
			
			
		//all this shit commented is just to help with positioning stuff onscreen
/*		else if (UBtn.GetState() == STATE_CLICKED)
		{
			UBtn.ResetState();
			if (btn)
			{
				progress1Img.SetPosition(progress1Img.GetLeft(),progress1Img.GetTop()-1);
			}
			
			else 
			{
				progress2Img.SetPosition(progress2Img.GetLeft(),progress2Img.GetTop()-1);
			}
		
			changed=true;
		
		}
		else if (DBtn.GetState() == STATE_CLICKED)
		{
			DBtn.ResetState();
			if (btn)
			{
				progress1Img.SetPosition(progress1Img.GetLeft(),progress1Img.GetTop()+1);
			}
			
			else 
			{
				progress2Img.SetPosition(progress2Img.GetLeft(),progress2Img.GetTop()+1);
			}
		
			changed=true;
		
		
		
		
		}
		else if (LBtn.GetState() == STATE_CLICKED)
		{
			LBtn.ResetState();
			if (btn)
			{
				progress1Img.SetPosition(progress1Img.GetLeft()-1,progress1Img.GetTop());
			}
			
			else 
			{
				progress2Img.SetPosition(progress2Img.GetLeft()-1,progress2Img.GetTop());
			}
		
			changed=true;
		
		
		
		
		}
		else if (RBtn.GetState() == STATE_CLICKED)
		{
			RBtn.ResetState();
			if (btn)
			{
				progress1Img.SetPosition(progress1Img.GetLeft()+1,progress1Img.GetTop());
			}
			
			else 
			{
				progress2Img.SetPosition(progress2Img.GetLeft()+1,progress2Img.GetTop());
			}
		
			changed=true;
		
		
		
		
		}*/
		else if (updateBtn.GetState() == STATE_CLICKED)
		{
			HaltGui();
			updateBtn.ResetState();
			//btn=!btn;
			//changed=true;
			pro=0;
			w->Remove(&playBtn);
			for (int i=0;i<NUMLINES;i++)
			{
				w->Append(infoTxt[i]);	
			}
			//mainWindow->SetBlackbox(true,CONSOLELEFT,0,screenwidth-CONSOLELEFT,500,(GXColor){0,0,0,100});
			//mainWindow->Append(bgConsoleImg);
			bgConsoleImg->SetAlpha(170);
			w->Remove(&updateBtn);
			w->Append(&progress1emptyImg);
			w->Append(&progress2emptyImg);
			w->Append(progress1fullImg);
			w->Append(&progress2fullImg);
			w->Append(&progress1Img);
			w->Append(&progress2Img);
			ResumeGui();
		
		
		}
		else if (playBtn.GetState() == STATE_CLICKED)
		{
		
			int choice = WindowPrompt("Brawl+ !","How do you want to boot?","DVD","IOS222 DVD","IOS222 USB");
			bool ret = false;
			if (choice==1)ret = cfg_save(false, false);
			else if (choice==2)ret = cfg_save(true,false);
			else if (choice==0)ret = cfg_save(true,true);
			if (ret)
			{
				play = true;
				menu = MENU_EXIT;
			}
			else WindowPrompt("SHIT!","An error just happened.  Check that your SD card is writable","ok");
		
		
		}
			
		/*if (changed)
		{
			snprintf(linebuf[6],sizeof(linebuf[6]),"(%i , %i)  (%i , %i)",progress1Img.GetLeft(),progress1Img.GetTop(),progress2Img.GetLeft(),progress2Img.GetTop());
			infoTxt[6]->SetText(linebuf[6]);
			
			gprintf("\n(%i , %i)  (%i , %i)",progress1Img.GetLeft(),progress1Img.GetTop(),progress2Img.GetLeft(),progress2Img.GetTop());
			changed =false;
		}*/
		else if (pro==0)
		{
			gprintf("\nInit network");
			updateList("Init network...",true);
			while (i>0 && !networkinitialized)
			{
				Initialize_Network();
				i--;
				sleep(1);
				if(ExitBtn->GetState() == STATE_CLICKED)break;
			
			}
			pro++;
			if(networkinitialized)
			{
				gprintf("%s",GetNetworkIP());
				snprintf(t,sizeof(t),"%s",GetNetworkIP());
				updateList(t,false);
				
			}
			
			else 
			{
				ex("Can't Init network");
			}
		}
		else if (pro==1)
		{
		
			gprintf("\n\tDownloading \"http://brawlplus.net/dl.php\"");
			updateList("Downloading http://brawlplus.net/dl.php",true);
			if (brawlDL("http://brawlplus.net/dl.php", "sd:/brawl+list.txt")){
				gprintf("\nsaved as \"sd:/brawl+list.txt\"");
				updateList("saved as sd:/brawl+list.txt",true);
			}
			else ex("Could not download the file");
			
			pro++;
			
		}
		else if (pro==2)
		{
			char line[400];
		
			
			FILE *f = fopen("sd:/brawl+list.txt", "r");
			if (f){//gprintf("\ncheck1");
				while (fgets(line, sizeof(line), f)) {
					numFiles++;//not really the number of files, but the nimber of lines in the text file
				}//gprintf("\ncheck2");
			}
			fclose(f);
			if (numFiles>0)
				{
					report = new customOptionList(numFiles-1);
					optionBrowser = new GuiCustomOptionBrowser(592, 280, report,  bg_options_settings_png, 1, 385,0);
					optionBrowser->SetPosition(20, 20);
					optionBrowser->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
			
					
				}
	//gprintf("\%i lines in the file",numFiles);
			pro++;
		}
		else if (pro==3)
		{
			i=0;
			char line[400];
			char buf[400];
			FILE *f = fopen("sd:/brawl+list.txt", "r");
			while (fgets(line, sizeof(line), f)) {
				u8 pass=0;
				memset(url,0,sizeof(url));
				memset(url2,0,sizeof(url2));
				memset(path,0,sizeof(path));
				memset(hash,0,sizeof(hash));
				
				
				int ll = parseLine1(line);
				if (ll>0)
				//if (parseLine(line,url,path,hash))
				{
					gprintf("\n\n<%i>----------------------------------------",fctn2+1);
					
					updateList(" ",true);
					sprintf(t,"<%i>------------------------------------------------------------------------------------------------------------------------",fctn2+1);
					updateList(t,true);
					
					
					//hashes[fctn] = new MD5;
					
					//if (fctn2%4==0)Con_Clear();
					memset(line,0,sizeof(line));		
					memset(buf,0,sizeof(buf));
					sprintf(buf,"sd:/%s",path);
					
					if (!checkfilehash(buf,hash))
					{
						makefolder(buf);
						
						
						sprintf(url2,"http://brawlplus.net/downloads/%s",url);
						gprintf("\nDownloading %s",url2);
						
						snprintf(t,sizeof(t),"Downloading %s",url2);
						updateList(t,true);
						
						if (brawlDL(url2, buf)){
							//gprintf("\nfile downloaded");
							if (!checkfilehash(buf,hash,true))
								{
									snprintf(t,sizeof(t),"Failed hash check after downloading, saving anyways");
									updateList(t,true);
									gprintf("\nFailed hash check after downloading, saving anyways");
									hashfails++;
									pass=0;
								}
							else 
							{
								updateList("Hash check passed",true);
								gprintf("\nHash check passed");
								pass=1;
							
							}
							
							
							
							fctn++;
						}	
						
					}
					else 
					{
						gprintf("\nno need to download file");
						updateList("no need to download file",true);
						pass=2;
					}
					////////////add entry to the report//////////

					report->SetValue(fctn2, "%s", ((pass==0?"Failed Hash check":(pass==1?"Updated OK":"No update Needed"))));
					report->SetName(fctn2, "%s",shortname(path));
					gprintf("\n%s -> %s",report->GetName(fctn2),report->GetValue(fctn2));
					fctn2++;
///////////////					
//			remove(buf);//delete files (for testing shit)
///////////////				
				}
				else if (ll<0)//this is an old list
				{
					gprintf("\nThe list is old --> ret = %i",ll);
					snprintf(t,sizeof(t),"The list is old --> ret = %i",ll);
					updateList(t,true);

					//fclose(f);
					break;
				}
				
				HaltGui();
				progress2fullImg.SetTile(40*i/numFiles);
				ResumeGui();
				i++;
				if(ExitBtn->GetState() == STATE_CLICKED)
				{
					menu = MENU_EXIT;
						break;
				}
///////////////				
//				if (i==10)break;//for testing shit
///////////////
			}
			fclose(f);
			gprintf("\n%i of %i files updated",fctn,fctn2);
			snprintf(t,sizeof(t),"%i of %i files updated",fctn,fctn2);
			updateList(t,true);
			


		
		
		
			pro++;
		}
		else if (pro==4)
		{
			HaltGui();
			for (int i=0;i<NUMLINES;i++)
			{
				w->Remove(infoTxt[i]);	
			}
			mainWindow->Remove(bgConsoleImg);
			//mainWindow->SetBlackbox(false,CONSOLELEFT,0,screenwidth-CONSOLELEFT,500,(GXColor){0,0,0,100});
			w->Remove(progress1fullImg);
			w->Remove(&progress2fullImg);
			w->Remove(&progress1Img);
			w->Remove(&progress2Img);
			w->Remove(&progress1emptyImg);
			w->Remove(&progress2emptyImg);
			w->Remove(&updateBtn);
			ResumeGui();
			
			snprintf(t,sizeof(t),"%i of %i files updated.  %i failed hash verification.",fctn,fctn2,hashfails);
			WindowPrompt(0,t,"OK");
			HaltGui();
			playBtn.SetPosition(0,308);
			w->Append(&playBtn);
			w->Append(optionBrowser);
			ResumeGui();
		
		
			pro++;
		}
		
	}
	HaltGui();
	mainWindow->Remove(w);
	if (optionBrowser)delete optionBrowser;
	if(report)delete report;
	for (int i=0;i<NUMLINES;i++)
	{
		delete(infoTxt[i]);	
	}
	infoTxt = NULL;
	
	ResumeGui();

	return menu;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
void MainMenu(int menu)
{
	int currentMenu = menu;
//	char tmp[100];

	#ifdef HW_RVL
	pointer[0] = new GuiImageData(player1_point_png);
	pointer[1] = new GuiImageData(player2_point_png);
	pointer[2] = new GuiImageData(player3_point_png);
	pointer[3] = new GuiImageData(player4_point_png);
	#endif

	mainWindow = new GuiWindow(screenwidth, screenheight);
	
	char iost[20];
	sprintf(iost,"Using IOS %u (v%u)",IOS_GetVersion(),IOS_GetRevision());
	GuiText iostxt(iost,16,(GXColor){255, 255, 0, 255});
	iostxt.SetPosition(20,-20);
	iostxt.SetAlignment(0,4);
	
	GuiText versiontxt(VERSIONTEXT,16,(GXColor){255, 255, 0, 255});
	versiontxt.SetPosition(20,-40);
	versiontxt.SetAlignment(0,4);
	
	GuiImageData bgData(background_png);
    bgImg = new GuiImage(&bgData);
	
	bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
	bgMusic->SetVolume(80);
	bgMusic->SetLoop(1);
	//bgMusic->Play(); // startup music
	
	for (float i=100.00;i>1;i-=1.0)
	{
		float scale = ((i/25)>=1?i/25:1);
		if (scale==1)break;
		float alpha = (2.55*(100-i))/2;
		Menu_DrawImg(-i+25, -0,bgImg->GetWidth(), bgImg->GetHeight(), bgImg->GetImage(), 0, scale, scale, alpha);
		Menu_Render();
		if (i==75)bgMusic->Play();
	}
	
	mainWindow->Append(bgImg);
	mainWindow->Append(&iostxt);
	mainWindow->Append(&versiontxt);
		
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	ResumeGui();

	while(currentMenu != MENU_EXIT)
	{
		switch (currentMenu)
		{
			case MENU_SELECT_IOS:
				currentMenu = MenuUpdate();
				break;
			default: // unrecognized menu
				currentMenu = MenuUpdate();
				break;
		}
	}
	
	ResumeGui();
	ExitGUIThreads();
	
	while(ExitRequested != 2) usleep(THREAD_SLEEP);

	bgMusic->Stop();
	delete bgMusic;
	delete bgImg;
	delete bgConsoleImg;
	delete mainWindow;
	delete pointer[0];
	delete pointer[1];
	delete pointer[2];
	delete pointer[3];
	
	delete progress1fullImg;
	delete ExitBtn;
	delete w;
	
	
	SDCard_deInit();
	usleep(500000);
	if (play) 
	{
		char loc[30];
		snprintf(loc,sizeof(loc),"sd:/apps/Gecko192/boot.elf");
		BootHomebrew(loc);
	}
	
	
	
    //last point in programm to make sure the allocated memory is freed
	Sys_BackToLoader();
}
