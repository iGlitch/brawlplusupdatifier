/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_text.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

static int currentSize = 0;
static int currentWidescreen = 0;
static int presetSize = 0;
static GXColor presetColor = (GXColor){255, 255, 255, 255};
static int presetMaxWidth = 0;
static int presetWrapMode = GuiText::WRAP;
static u16 presetStyle = FTGX_NULL;
static int presetAlignmentHor = 0;
static int presetAlignmentVert = 0;

/**
 * Constructor for the GuiText class.
 */
GuiText::GuiText(const char * t, int s, GXColor c)
{
	text = NULL;
	size = s;
	color = c;
	alpha = c.a;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	wrapMode = GuiText::WRAP;
	scrollPos1 = 0;
	scrollPos2 = 0;
	scrollDelay = 0;
	font = NULL;
	widescreen = 0; //added

	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;

	if(t)
		text = FreeTypeGX::charToWideChar((char *)t);
}

/**
 * Constructor for the GuiText class, uses presets
 */
GuiText::GuiText(const char * t)
{
	text = NULL;
	size = presetSize;
	color = presetColor;
	alpha = presetColor.a;
	style = presetStyle;
	maxWidth = presetMaxWidth;
	wrapMode = presetWrapMode;
	scrollPos1 = 0;
	scrollPos2 = 0;
	scrollDelay = 0;
	font = NULL;
	widescreen = 0; //added

	alignmentHor = presetAlignmentHor;
	alignmentVert = presetAlignmentVert;

	if(t)
		text = FreeTypeGX::charToWideChar((char *)t);
}

/**
 * Destructor for the GuiText class.
 */
GuiText::~GuiText()
{
	if(text)
	{
		delete [] text;
		text = NULL;
	}
}

void GuiText::SetText(const char * t)
{
	if(text)
		delete [] text;
	text = NULL;

	if(t)
		text = FreeTypeGX::charToWideChar((char *)t);
	scrollPos2 = 0;
	scrollDelay = 0;
}
void GuiText::SetTextf(const char *format, ...)
{
	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		this->SetText(tmp);
		free(tmp);
	}
	va_end(va);
}

void GuiText::SetPresets(int sz, GXColor c, int w, int wrap, u16 s, int h, int v)
{
	presetSize = sz;
	presetColor = c;
	presetMaxWidth = w;
	presetWrapMode = wrap;
	presetStyle = s;
	presetAlignmentHor = h;
	presetAlignmentVert = v;
}

void GuiText::SetFontSize(int s)
{
	size = s;
}

void GuiText::SetMaxWidth(int w, short m/*=GuiText::WRAP*/)
{
	maxWidth = w;
	wrapMode = m;
}

void GuiText::SetColor(GXColor c)
{
	color = c;
	alpha = c.a;
}

void GuiText::SetStyle(u16 s, u16 m/*=0xffff*/)
{
	style &= ~m;
	style |= s & m;
}

void GuiText::SetAlignment(int hor, int vert)
{
	style = FTGX_NULL;

	switch(hor)
	{
		case ALIGN_LEFT:
			style |= FTGX_JUSTIFY_LEFT;
			break;
		case ALIGN_RIGHT:
			style |= FTGX_JUSTIFY_RIGHT;
			break;
		default:
			style |= FTGX_JUSTIFY_CENTER;
			break;
	}
	switch(vert)
	{
		case ALIGN_TOP:
			style |= FTGX_ALIGN_TOP;
			break;
		case ALIGN_BOTTOM:
			style |= FTGX_ALIGN_BOTTOM;
			break;
		default:
			style |= FTGX_ALIGN_MIDDLE;
			break;
	}

	alignmentHor = hor;
	alignmentVert = vert;
}
/**
 * Set the Font
 */
void GuiText::SetFont(FreeTypeGX *f)
{
	font = f;
}

int GuiText::GetTextWidth()
{
	if(!text)
		return 0;

	return fontSystem->getWidth(text);
}

void GuiText::SetWidescreen(bool w)
{
	widescreen = w;
}
/**
 * Draw the text on screen
 */
void GuiText::Draw()
{
	if(!text)
		return;

	if(!this->IsVisible())
		return;

	GXColor c = color;
	c.a = this->GetAlpha();

	int newSize = size*this->GetScale();

	if(newSize != currentSize)
	{
		//fontSystem->changeSize(newSize);
		(font ? font : fontSystem)->changeSize(newSize, 0);
		currentSize = newSize;
		currentWidescreen = widescreen;
	}

	int voffset = 0;

//	if(alignmentVert == ALIGN_MIDDLE)
//		voffset = -newSize/2 + 2;

	if(maxWidth > 0 && (font ? font : fontSystem)->getWidth(text) > maxWidth)
	{
		if(wrapMode == GuiText::WRAP) // text wrapping
		{
			int lineheight = newSize + 6;
			int strlen = wcslen(text);
			int i = 0;
			int ch = 0;
			int linenum = 0;
			int lastSpace = -1;
			int lastSpaceIndex = -1;
			wchar_t * tmptext[20];

			while(ch < strlen)
			{
				if(i == 0)
					tmptext[linenum] = new wchar_t[strlen + 1];

				tmptext[linenum][i] = text[ch];
				tmptext[linenum][i+1] = 0;

				if(text[ch] == ' ' || ch == strlen-1)
				{
					if((font ? font : fontSystem)->getWidth(tmptext[linenum]) >= maxWidth)
					//if(fontSystem->getWidth(tmptext[linenum]) >= maxWidth)
					{
						if(lastSpace >= 0)
						{
							tmptext[linenum][lastSpaceIndex] = 0; // discard space, and everything after
							ch = lastSpace; // go backwards to the last space
							lastSpace = -1; // we have used this space
							lastSpaceIndex = -1;
						}
						linenum++;
						i = -1;
					}
					else if(ch == strlen-1)
					{
						linenum++;
					}
				}
				if(text[ch] == ' ' && i >= 0)
				{
					lastSpace = ch;
					lastSpaceIndex = i;
				}
				ch++;
				i++;
			}

			if(alignmentVert == ALIGN_MIDDLE)
				voffset = voffset - (lineheight*linenum)/2 + lineheight/2;

			for(i=0; i < linenum; i++)
			{
				(font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop()+voffset+i*lineheight, tmptext[i], c, style);
				delete tmptext[i];
			}
		}
		else if(wrapMode == GuiText::DOTTED) // text dotted
		{
			wchar_t save[4];
			int strlen = wcslen(text);
			int dotPos=strlen-3;
			int i;
			bool drawed = false;
			while(dotPos > 0 && drawed == false)
			{
				for(i=0; i<4; i++) // save Text for "..."
				{
					save[i] = text[dotPos+i];
					text[dotPos+i] = (i != 3 ? _TEXT('.') : 0);
				}
				if(((font ? font : fontSystem)->getWidth(text)) <= maxWidth)
				{
					(font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop()+voffset, text, c, style);
					drawed = true;
				}

				for(i=0; i<4; i++) // write saved Text back
					text[dotPos+i] = save[i];
				dotPos--;
			}
			if(!drawed)
				(font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop()+voffset, text, c, style);
		}
		else if(wrapMode == GuiText::SCROLL) // text scroller
		{
			wchar_t save;

			if(scrollPos2 == 0 || frameCount > scrollDelay+5)
			{
				scrollPos1 = 0;
				scrollOffset = 0;
				for(scrollPos2 = wcslen(text); scrollPos2 > 1; scrollPos2--)
				{
					save = text[scrollPos2]; // save Pos2
					text[scrollPos2] = 0;
					int textWidth = (font ? font : fontSystem)->getWidth(text);
					text[scrollPos2] = save; // restore Pos2
					if(textWidth <= maxWidth)
						break;
				}
				scrollDelay = frameCount+50; // wait 50 Frames before beginning with scrolling
			}
			else if(scrollPos2 > 0 && frameCount >= scrollDelay)
			{

				if(--scrollOffset < 0)
				{
					wchar_t tmp[] = { text[scrollPos1], text[scrollPos1+1], 0 };
					scrollOffset += (font ? font : fontSystem)->getWidth(tmp) - (font ? font : fontSystem)->getWidth(tmp+1);
					scrollPos1++;
				}

				int strlen = wcslen(text);
				for(; scrollPos2 < strlen; scrollPos2++)
				{
					save = text[scrollPos2+1]; // save Pos2
					text[scrollPos2+1] = 0;
					int textWidth = (font ? font : fontSystem)->getWidth(&text[scrollPos1]);
					text[scrollPos2+1] = save; // restore Pos2
					if(textWidth+scrollOffset > maxWidth)
						break;
				}
				if(scrollPos2 == strlen)
				{
					scrollPos2 = -scrollPos2;
					scrollDelay = frameCount+25; // when dir-change wait 25 Frames
				}
				else
					scrollDelay = frameCount+1; // wait 1 Frames
			}
			else if(frameCount >= scrollDelay)
			{
				scrollPos2 = -scrollPos2;

				scrollOffset++;
				wchar_t tmp[] = { text[scrollPos1-1], text[scrollPos1], 0 };
				int tmpOffset = (font ? font : fontSystem)->getWidth(tmp) - (font ? font : fontSystem)->getWidth(tmp+1);
				if(scrollOffset >= tmpOffset)
				{
					scrollOffset -= tmpOffset;
					scrollPos1--;
				}

				for(; scrollPos2 > scrollPos1; scrollPos2--)
				{
					save = text[scrollPos2]; // save Pos2
					text[scrollPos2] = 0;
					int textWidth = (font ? font : fontSystem)->getWidth(&text[scrollPos1]);
					text[scrollPos2] = save; // restore Pos2
					if(textWidth+scrollOffset <= maxWidth)
						break;
				}
				if(scrollPos1 == 0)
				{
					scrollPos2 = -scrollPos2;
					scrollDelay = frameCount+25; // when dir-change wait 25 Frames
				}
				else
					scrollDelay = frameCount+1; // wait 10 Frames

				scrollPos2 = -scrollPos2;
			}

			uint16_t drawStyle = style;
			uint16_t drawX = this->GetLeft() + scrollOffset;

			if((drawStyle & FTGX_JUSTIFY_MASK) == FTGX_JUSTIFY_CENTER)
			{
				drawStyle = (drawStyle & ~FTGX_JUSTIFY_MASK) | FTGX_JUSTIFY_LEFT;
				drawX -= maxWidth >> 1;
			}
			else if((drawStyle & FTGX_JUSTIFY_MASK) == FTGX_JUSTIFY_RIGHT)
			{
				drawStyle = (drawStyle & ~FTGX_JUSTIFY_MASK) | FTGX_JUSTIFY_LEFT;
				drawX -= maxWidth;
			}

			save = text[abs(scrollPos2)]; // save Pos2
			text[abs(scrollPos2)] = 0;
			(font ? font : fontSystem)->drawText(drawX, this->GetTop()+voffset, &text[scrollPos1], c, drawStyle);
			text[abs(scrollPos2)] = save; // restore Pos2
		}
	}
	else
	{
		(font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop()+voffset, text, c, style);
	}
	this->UpdateEffects();
}
