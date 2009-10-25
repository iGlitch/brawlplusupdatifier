/****************************************************************************
 * gui_optionbrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "gui_optionbrowser.h"

#define BROWSERSIZE        8


OptionList::OptionList(int size)
{
	name = new char * [size+1];
	value = new char * [size+1];
	for (int i = 0; i < size+1; i++)
	{
		name[i] = 0;
		value[i] = 0;
	}
	length = size+1;
	changed = false;
}
OptionList::~OptionList()
{
	for (int i = 0; i < length; i++)
	{
		free(name[i]);
		free(value[i]);
	}
	delete [] name;
	delete [] value;
}

void OptionList::SetName(int i, const char *format, ...)
{
	if(i >= 0 && i < length)
	{
		if(name[i]) free(name[i]);
		name[i] = NULL;
		va_list va;
		va_start(va, format);
		vasprintf(&name[i], format, va);
		va_end(va);
		changed = true;
	}
}
void OptionList::SetValue(int i, const char *format, ...)
{
	if(i >= 0 && i < length)
	{
		char *tmp = NULL;
		va_list va;
		va_start(va, format);
		vasprintf(&tmp, format, va);
		va_end(va);

		if(tmp)
		{
			if(value[i] && !strcmp(tmp, value[i]))
				free(tmp);
			else
			{
				free(value[i]);
				value[i] = tmp;
				changed = true;
			}
		}
	}
}

/**
 * Constructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::GuiOptionBrowser(int w, int h, OptionList * l)
{
	width = w;
	height = h;
	options = l;
	size = ((l->GetLength() > BROWSERSIZE) ? BROWSERSIZE: l->GetLength());
	scrollbaron = (l->GetLength() < BROWSERSIZE) ? 0 : 1;
	selectable = true;
	listOffset = this->FindMenuItem(-1, 1);
	selectedItem = 0;
	focus = 1; // allow focus
	coL2 = 100;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A, PAD_BUTTON_A);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	bgOptions = new GuiImageData(bg_browser_png);
	bgOptionsImg = new GuiImage(bgOptions);
	bgOptionsImg->SetParent(this);
	bgOptionsImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	bgOptionsEntry = new GuiImageData(bg_browser_selection_png);

	scrollbar = new GuiImageData(scrollbar_png);
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetParent(this);
	scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarImg->SetPosition(0, 30);

	arrowDown = new GuiImageData(scrollbar_arrowdown_png);
	arrowDownImg = new GuiImage(arrowDown);

	arrowDownOver = new GuiImageData(scrollbar_arrowdown_png);
	arrowDownOverImg = new GuiImage(arrowDownOver);

	arrowUp = new GuiImageData(scrollbar_arrowup_png);
	arrowUpImg = new GuiImage(arrowUp);

	arrowUpOver = new GuiImageData(scrollbar_arrowup_png);
	arrowUpOverImg = new GuiImage(arrowUpOver);

	scrollbarBox = new GuiImageData(scrollbar_box_png);
	scrollbarBoxImg = new GuiImage(scrollbarBox);

	scrollbarBoxOver = new GuiImageData(scrollbar_box_png);
	scrollbarBoxOverImg = new GuiImage(scrollbarBoxOver);

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetParent(this);
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetImageOver(arrowUpOverImg);
	arrowUpBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	arrowUpBtn->SetSelectable(false);
	arrowUpBtn->SetClickable(false);
	arrowUpBtn->SetHoldable(true);
	arrowUpBtn->SetTrigger(trigHeldA);
	arrowUpBtn->SetSoundClick(btnSoundClick);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetParent(this);
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetImageOver(arrowDownOverImg);
	arrowDownBtn->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	arrowDownBtn->SetSelectable(false);
	arrowDownBtn->SetClickable(false);
	arrowDownBtn->SetHoldable(true);
	arrowDownBtn->SetTrigger(trigHeldA);
	arrowDownBtn->SetSoundClick(btnSoundClick);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
	scrollbarBoxBtn->SetParent(this);
	scrollbarBoxBtn->SetImage(scrollbarBoxImg);
	scrollbarBoxBtn->SetImageOver(scrollbarBoxOverImg);
	scrollbarBoxBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarBoxBtn->SetMinY(0);
	scrollbarBoxBtn->SetMaxY(136);
	scrollbarBoxBtn->SetSelectable(false);
	scrollbarBoxBtn->SetClickable(false);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldA);

	optionIndex = new int[size];
	optionVal = new GuiText * [size];
	optionValOver = new GuiText * [size];
	optionBtn = new GuiButton * [size];
	optionTxt = new GuiText * [size];
	optionBg = new GuiImage * [size];

	for(int i=0; i < size; i++)
	{
		optionTxt[i] = new GuiText(options->GetName(i), 20, (GXColor){0, 0, 0, 0xff});
		optionTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionTxt[i]->SetPosition(24,0);
		optionTxt[i]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2+24), GuiText::DOTTED);

		optionBg[i] = new GuiImage(bgOptionsEntry);
		optionBg[i]->SetPosition(-5, 0);

		optionVal[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionVal[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

		optionValOver[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionValOver[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

		optionBtn[i] = new GuiButton(width-28, 32);
		optionBtn[i]->SetParent(this);
		optionBtn[i]->SetLabel(optionTxt[i], 0);
		optionBtn[i]->SetLabel(optionVal[i], 1);
		optionBtn[i]->SetLabelOver(optionValOver[i], 1);
		optionBtn[i]->SetImageOver(optionBg[i]);
		optionBtn[i]->SetPosition(10, 32*i+4);
		optionBtn[i]->SetRumble(false);
		optionBtn[i]->SetTrigger(trigA);
		optionBtn[i]->SetSoundClick(btnSoundClick);

	}
	UpdateListEntries();
}

/**
 * Destructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::~GuiOptionBrowser()
{
	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;
	delete scrollbarImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;
	delete scrollbarBoxImg;
	delete scrollbarBoxOverImg;
	delete scrollbar;
	delete arrowDown;
	delete arrowDownOver;
	delete arrowUp;
	delete arrowUpOver;
	delete scrollbarBox;
	delete scrollbarBoxOver;

    delete bgOptionsImg;
	delete bgOptions;
	delete bgOptionsEntry;

	delete trigA;
	delete trigHeldA;
	delete btnSoundClick;

	for(int i = 0; i < size; i++)
	{
		delete optionTxt[i];
		delete optionVal[i];
		delete optionValOver[i];
		delete optionBg[i];
		delete optionBtn[i];
	}
	delete [] optionIndex;
	delete [] optionVal;
	delete [] optionValOver;
	delete [] optionBtn;
	delete [] optionTxt;
	delete [] optionBg;
}

void GuiOptionBrowser::SetFocus(int f)
{
	focus = f;

	for(int i = 0; i < size; i++)
		optionBtn[i]->ResetState();

	if(f == 1)
		optionBtn[selectedItem]->SetState(STATE_SELECTED);
}

void GuiOptionBrowser::ResetState()
{
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for(int i = 0; i < size; i++)
	{
		optionBtn[i]->ResetState();
	}
}

int GuiOptionBrowser::GetClickedOption()
{
	int found = -1;
	for(int i = 0; i < size; i++)
	{
		if(optionBtn[i]->GetState() == STATE_CLICKED)
		{
			optionBtn[i]->SetState(STATE_SELECTED);
			found = optionIndex[i];
			break;
		}
	}
	return found;
}

int GuiOptionBrowser::GetSelectedOption()
{
	int found = -1;
	for(int i = 0; i < size; i++)
	{
		if(optionBtn[i]->GetState() == STATE_SELECTED)
		{
			found = optionIndex[i];
			break;
		}
	}
	return found;
}

void GuiOptionBrowser::SetClickable(bool enable)
{
	for(int i = 0; i < size; i++)
	{
		optionBtn[i]->SetClickable(enable);
	}
}

void GuiOptionBrowser::SetScrollbar(int enable)
{
	scrollbaron = enable;
}

void GuiOptionBrowser::SetOffset(int optionnumber)
{
    listOffset = optionnumber;
    selectedItem = optionnumber;
}

/****************************************************************************
 * FindMenuItem
 *
 * Help function to find the next visible menu item on the list
 ***************************************************************************/

int GuiOptionBrowser::FindMenuItem(int currentItem, int direction)
{
	int nextItem = currentItem + direction;

	if(nextItem < 0 || nextItem >= options->GetLength())
		return -1;

	if(strlen(options->GetName(nextItem)) > 0)
		return nextItem;
	else
		return FindMenuItem(nextItem, direction);
}

/**
 * Draw the button on screen
 */
void GuiOptionBrowser::Draw()
{
	if(!this->IsVisible())
		return;

	bgOptionsImg->Draw();

	int next = listOffset;

	for(int i=0; i < size; i++)
	{
		if(next >= 0)
		{
			optionBtn[i]->Draw();
			next = this->FindMenuItem(next, 1);
		}
		else
			break;
	}

    if(scrollbaron == 1) {
        scrollbarImg->Draw();
        arrowUpBtn->Draw();
        arrowDownBtn->Draw();
        scrollbarBoxBtn->Draw();
    }
	this->UpdateEffects();
}

void GuiOptionBrowser::UpdateListEntries()
{
	if(listOffset<0) listOffset = this->FindMenuItem(-1, 1);
	int next = listOffset;

	int maxNameWidth = 0;
	for(int i=0; i < size; i++)
	{
		if(next >= 0)
		{
			if(optionBtn[i]->GetState() == STATE_DISABLED)
			{
				optionBtn[i]->SetVisible(true);
				optionBtn[i]->SetState(STATE_DEFAULT);
			}

			optionTxt[i]->SetText(options->GetName(next));
			if(maxNameWidth < optionTxt[i]->GetTextWidth())
				maxNameWidth = optionTxt[i]->GetTextWidth();
			optionVal[i]->SetText(options->GetValue(next));
			optionValOver[i]->SetText(options->GetValue(next));

			optionIndex[i] = next;
			next = this->FindMenuItem(next, 1);
		}
		else
		{
			optionBtn[i]->SetVisible(false);
			optionBtn[i]->SetState(STATE_DISABLED);
		}
	}
	if(coL2 < (24+maxNameWidth+16))
		coL2 = 24+maxNameWidth+16;
	for(int i=0; i < size; i++)
	{
		if(optionBtn[i]->GetState() != STATE_DISABLED)
		{
			optionVal[i]->SetPosition(coL2,0);
			optionVal[i]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2+24), GuiText::DOTTED);

			optionValOver[i]->SetPosition(coL2,0);
			optionValOver[i]->SetMaxWidth(bgOptionsImg->GetWidth() - (coL2+24), GuiText::SCROLL);
		}
	}
}

void GuiOptionBrowser::Update(GuiTrigger * t)
{

	if(state == STATE_DISABLED || !t)
		return;


	int next, prev, lang = options->GetLength(), positionWiimote = 0;

	if(options->IsChanged()) {
		coL2 = 0;
		UpdateListEntries();
	}
	int old_listOffset = listOffset;

    if (scrollbaron == 1)
	{
		// update the location of the scroll box based on the position in the option list
		arrowUpBtn->Update(t);
		arrowDownBtn->Update(t);
		scrollbarBoxBtn->Update(t);
    }

	next = listOffset;

    if((t->wpad.btns_h != WPAD_BUTTON_UP) && (t->wpad.btns_h != WPAD_BUTTON_DOWN)) {
        for(int i=0; i < size; i++)
        {
		if(next >= 0)
			next = this->FindMenuItem(next, 1);

		if(focus)
		{
			if(i != selectedItem && optionBtn[i]->GetState() == STATE_SELECTED) {
				optionBtn[i]->ResetState();
			} else if(i == selectedItem && optionBtn[i]->GetState() == STATE_DEFAULT) {
				optionBtn[selectedItem]->SetState(STATE_SELECTED);
			}
		}


		optionBtn[i]->Update(t);

		if(optionBtn[i]->GetState() == STATE_SELECTED)
		{
			selectedItem = i;
		}
        }
	}

	// pad/joystick navigation
	if(!focus)
		return; // skip navigation

    if (scrollbaron == 1)
	{
        if(t->Right())
		{
			if(listOffset < lang && lang > size)
			{
				listOffset =listOffset+ size;
				if(listOffset+size >= lang)
				listOffset = lang-size;
			}
		}
		else if(t->Left())
		{
			if(listOffset > 0)
			{
				listOffset =listOffset- size;
				if(listOffset < 0)
					listOffset = 0;
			}
		}

	    if(arrowDownBtn->GetState() == STATE_HELD && arrowDownBtn->GetStateChan() == t->chan)
        {
            t->wpad.btns_h |= WPAD_BUTTON_DOWN;
            if(!this->IsFocused())
                ((GuiWindow *)this->GetParent())->ChangeFocus(this);
        }
        else if(arrowUpBtn->GetState() == STATE_HELD && arrowUpBtn->GetStateChan() == t->chan)
        {
            t->wpad.btns_h |= WPAD_BUTTON_UP;
            if(!this->IsFocused())
                ((GuiWindow *)this->GetParent())->ChangeFocus(this);
        }

        if(scrollbarBoxBtn->GetState() == STATE_HELD &&
		scrollbarBoxBtn->GetStateChan() == t->chan &&
		t->wpad.ir.valid && lang > BROWSERSIZE)
        {
            scrollbarBoxBtn->SetPosition(0,0);
            positionWiimote = t->wpad.ir.y - 60 - scrollbarBoxBtn->GetTop();

            if(positionWiimote < scrollbarBoxBtn->GetMinY())
                positionWiimote = scrollbarBoxBtn->GetMinY();
            else if(positionWiimote > scrollbarBoxBtn->GetMaxY())
                positionWiimote = scrollbarBoxBtn->GetMaxY();

            listOffset = (positionWiimote * lang)/136.0 - selectedItem;

            if(listOffset <= 0)
            {
                listOffset = 0;
            }
            else if(listOffset+BROWSERSIZE >= lang)
            {
                listOffset = lang-BROWSERSIZE;
            }
            focus = false;
        }

		if(scrollbarBoxBtn->GetState() == STATE_HELD &&
			scrollbarBoxBtn->GetStateChan() == t->chan &&
			t->wpad.ir.valid && options->GetLength() > size)
		{
			scrollbarBoxBtn->SetPosition(width/2-18+7,0);

			int position = t->wpad.ir.y - 50 - scrollbarBoxBtn->GetTop();

			listOffset = (position * lang)/180 - selectedItem;

			if(listOffset <= 0) {
				listOffset = 0;
				selectedItem = 0;
			} else if(listOffset+size >= lang) {
				listOffset = lang-size;
				selectedItem = size-1;
			}
		}

		int position;

		if(positionWiimote > 0)
        {
            position = positionWiimote; // follow wiimote cursor
        }
        else
        {
            position = 136*(listOffset + BROWSERSIZE/2.0) / (lang*1.0);

            if(listOffset/(BROWSERSIZE/2.0) < 1)
                position = 0;
            else if((listOffset+BROWSERSIZE)/(BROWSERSIZE*1.0) >= (lang)/(BROWSERSIZE*1.0))
                position = 136;
        }

        scrollbarBoxBtn->SetPosition(0,position+36);


		if(t->Right())
		{
			if(listOffset < lang && lang > size)
			{
				listOffset =listOffset+ size;
				if(listOffset+size >= lang)
				listOffset = lang-size;
			}
		}
		else if(t->Left())
		{
			if(listOffset > 0)
			{
				listOffset =listOffset- size;
				if(listOffset < 0)
					listOffset = 0;
			}
		}
    } else {

        if(t->Right())
		{
			if(listOffset < lang && lang > size)
			{
				listOffset =listOffset+ size;
				if(listOffset+size >= lang)
				listOffset = lang-size;
			}
		}
		else if(t->Left())
		{
			if(listOffset > 0)
			{
				listOffset =listOffset- size;
				if(listOffset < 0)
					listOffset = 0;
			}
		}
        else if(t->Down())
		{
			next = this->FindMenuItem(optionIndex[selectedItem], 1);

			if(next >= 0)
			{
				if(selectedItem == size-1)
				{
					// move list down by 1
					listOffset = this->FindMenuItem(listOffset, 1);
				}
				else if(optionBtn[selectedItem+1]->IsVisible())
				{
					optionBtn[selectedItem]->ResetState();
					optionBtn[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
					selectedItem++;
				}
			}
		}
		else if(t->Up())
		{
			prev = this->FindMenuItem(optionIndex[selectedItem], -1);

			if(prev >= 0)
			{
				if(selectedItem == 0)
				{
					// move list up by 1
					listOffset = prev;
				}
				else
				{
					optionBtn[selectedItem]->ResetState();
					optionBtn[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
					selectedItem--;
				}
			}
		}
    }

	if(old_listOffset != listOffset)
		UpdateListEntries();

	if(updateCB)
		updateCB(this);
}
