/*
Copyright (c) 2010, Sean Kasun
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"
#include <CommDlg.h>
#include "Resource.h"
#include "../MinutorMap/blockInfo.h"
#include "ColorSchemes.h"


#define COLORKEY L"Software\\Sean Kasun\\Minutor\\ColorSchemes"

ColorManager::ColorManager()
{
	RegCreateKeyEx(HKEY_CURRENT_USER,COLORKEY,0,NULL,
		REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,NULL,&key,NULL);
}
ColorManager::~ColorManager()
{
	RegCloseKey(key);
}

void ColorManager::Init(ColorScheme *cs)
{
	for (int i=0;i<numBlocks;i++)
	{
		unsigned int color=blocks[i].color;
		unsigned char r,g,b,a;
		r=color>>16;
		g=color>>8;
		b=color;
		double alpha=blocks[i].alpha;
		r=(unsigned char)(r/alpha);
		g=(unsigned char)(g/alpha);
		b=(unsigned char)(b/alpha);
		a=(unsigned char)(alpha*255);
		color=(r<<24)|(g<<16)|(b<<8)|a;
		cs->colors[i]=color;
	}
}
void ColorManager::create(ColorScheme *cs)
{
	DWORD schemeId,len;
	len=sizeof(schemeId);
	long result=RegQueryValueEx(key,L"schemeId",NULL,NULL,(LPBYTE)&schemeId,&len);
	if (result==ERROR_FILE_NOT_FOUND)
		schemeId=0;
	schemeId++;
	RegSetValueEx(key,L"schemeId",NULL,REG_DWORD,(LPBYTE)&schemeId,sizeof(schemeId));
	ColorManager::Init(cs);
	cs->id=schemeId;
	save(cs);
}
int ColorManager::next(int id,ColorScheme *cs)
{
	TCHAR name[50];
	DWORD nameLen;
	DWORD type,csLen;
	do {
		nameLen=50;
		csLen=sizeof(ColorScheme);
		LONG result=RegEnumValue(key,id,name,&nameLen,NULL,&type,(LPBYTE)cs,&csLen);
		id++;
		if (result==ERROR_NO_MORE_ITEMS)
			return 0;
		if (type==REG_BINARY)
			return id;
	} while (1);
}
void ColorManager::save(ColorScheme *cs)
{
	wchar_t keyname[50];
	swprintf(keyname,50,L"scheme %d",cs->id);
	RegSetValueEx(key,keyname,NULL,REG_BINARY,(LPBYTE)cs,sizeof(ColorScheme));
}
void ColorManager::load(ColorScheme *cs)
{
	wchar_t keyname[50];
	swprintf(keyname,50,L"scheme %d",cs->id);
	DWORD csLen=sizeof(ColorScheme);
	RegQueryValueEx(key,keyname,NULL,NULL,(LPBYTE)cs,&csLen);
}
void ColorManager::remove(int id)
{
	wchar_t keyname[50];
	swprintf(keyname,50,L"scheme %d",id);
	RegDeleteValue(key,keyname);
}

INT_PTR CALLBACK ColorSchemes(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK ColorSchemeEdit(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);

void doColorSchemes(HINSTANCE hInst,HWND hWnd)
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_COLORSCHEMES),hWnd,ColorSchemes);
}

static void validateButtons(HWND hDlg);
static ColorScheme curCS;

INT_PTR CALLBACK ColorSchemes(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	HWND list;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			ColorManager cm;
			list=GetDlgItem(hDlg,IDC_SCHEMELIST);
			int id=0;
			ColorScheme cs;
			while (id=cm.next(id,&cs))
			{
				int pos=SendMessage(list,LB_ADDSTRING,0,(LPARAM)cs.name);
				SendMessage(list,LB_SETITEMDATA,pos,cs.id);
			}
			validateButtons(hDlg);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SCHEMELIST:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			case LBN_SELCANCEL:
				validateButtons(hDlg);
				break;
			}
			break;
		case IDC_EDIT:
			{
				list=GetDlgItem(hDlg,IDC_SCHEMELIST);
				int item=(int)SendMessage(list,LB_GETCURSEL,0,0);
				curCS.id=(int)SendMessage(list,LB_GETITEMDATA,item,0);
				ColorManager cm;
				cm.load(&curCS);
				DialogBox(NULL,MAKEINTRESOURCE(IDD_COLORSCHEME),hDlg,ColorSchemeEdit);
				cm.save(&curCS);
				SendMessage(list,LB_DELETESTRING,item,0);
				int pos=(int)SendMessage(list,LB_INSERTSTRING,item,(LPARAM)curCS.name);
				SendMessage(list,LB_SETITEMDATA,pos,curCS.id);
			}
			break;
		case IDC_ADD:
			{
				ColorManager cm;
				ColorScheme cs;
				wcsncpy_s(cs.name,255,L"Color Scheme",255);
				cm.create(&cs);
				list=GetDlgItem(hDlg,IDC_SCHEMELIST);
				int pos=SendMessage(list,LB_ADDSTRING,0,(LPARAM)cs.name);
				SendMessage(list,LB_SETITEMDATA,pos,cs.id);
				validateButtons(hDlg);
			}
			break;
		case IDC_REMOVE:
			{
				ColorManager cm;
				list=GetDlgItem(hDlg,IDC_SCHEMELIST);
				int item=(int)SendMessage(list,LB_GETCURSEL,0,0);
				if (item!=LB_ERR)
				{
					int id=(int)SendMessage(list,LB_GETITEMDATA,item,0);
					cm.remove(id);
					SendMessage(list,LB_DELETESTRING,item,0);
				}
				validateButtons(hDlg);
			}
			break;
		case IDCANCEL:
		case IDOK:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
static void validateButtons(HWND hDlg)
{
	HWND edit=GetDlgItem(hDlg,IDC_EDIT);
	HWND remove=GetDlgItem(hDlg,IDC_REMOVE);
	HWND list=GetDlgItem(hDlg,IDC_SCHEMELIST);
	int item=(int)SendMessage(list,LB_GETCURSEL,0,0);
	if (item==LB_ERR)
	{
		EnableWindow(edit,FALSE);
		EnableWindow(remove,FALSE);
	}
	else
	{
		EnableWindow(edit,TRUE);
		EnableWindow(remove,TRUE);
	}
}

INT_PTR CALLBACK ColorSchemeEdit(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	static int curSel;
	static wchar_t row[255];
	NMLVDISPINFO *info;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			SetDlgItemText(hDlg,IDC_SCHEMENAME,curCS.name);
			
			HWND ctl=GetDlgItem(hDlg,IDC_CURCOLOR);
			EnableWindow(ctl,FALSE);
			ctl=GetDlgItem(hDlg,IDC_CURALPHA);
			EnableWindow(ctl,FALSE);

			HWND lv=GetDlgItem(hDlg,IDC_COLORLIST);
			ListView_SetExtendedListViewStyle(lv,LVS_EX_FULLROWSELECT);
			
			LVCOLUMN lvc;
			lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
			for (int i=0;i<4;i++)
			{
				lvc.iSubItem=i;
				switch (i)
				{
				case 0:
					lvc.pszText=L"Id";
					lvc.cx=30;
					break;
				case 1:
					lvc.pszText=L"Name";
					lvc.cx=200;
					break;
				case 2:
					lvc.pszText=L"Color";
					lvc.cx=80;
					break;
				case 3:
					lvc.pszText=L"Alpha";
					lvc.cx=50;
					break;
				}
				lvc.fmt=LVCFMT_LEFT;
				ListView_InsertColumn(lv,i,&lvc);
			}
			LVITEM item;
			item.mask=LVIF_TEXT|LVIF_STATE;
			item.iSubItem=0;
			item.state=0;
			item.stateMask=0;
			item.pszText=LPSTR_TEXTCALLBACK;
			for (int i=0;i<numBlocks;i++)
			{
				item.iItem=i;
				ListView_InsertItem(lv,&item);
			}
		}
		return (INT_PTR)TRUE;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW item=(LPNMLISTVIEW)lParam;
				if (item->uNewState&LVIS_SELECTED)
				{
					curSel=item->iItem;
					swprintf(row,255,L"#%06x",curCS.colors[item->iItem]>>8);
					SetDlgItemText(hDlg,IDC_CURCOLOR,row);
					swprintf(row,255,L"%d",curCS.colors[item->iItem]&0xff);
					SetDlgItemText(hDlg,IDC_CURALPHA,row);
					HWND ctl=GetDlgItem(hDlg,IDC_CURCOLOR);
					EnableWindow(ctl,TRUE);
					ctl=GetDlgItem(hDlg,IDC_CURALPHA);
					EnableWindow(ctl,TRUE);
				}
			}
			break;
		case LVN_GETDISPINFO:
			info=(NMLVDISPINFO*)lParam;
			switch (info->item.iSubItem)
			{
			case 0:
				swprintf(row,255,L"%d.",info->item.iItem);
				break;
			case 1:
				swprintf(row,255,L"%S",blocks[info->item.iItem].name);
				break;
			case 2:
				swprintf(row,255,L"#%06x",curCS.colors[info->item.iItem]>>8);
				break;
			case 3:
				swprintf(row,255,L"%d",curCS.colors[info->item.iItem]&0xff);
				break;
			}
			info->item.pszText=row;
			break;
		}
		break;
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case EN_CHANGE:
			switch (LOWORD(wParam))
			{
			case IDC_SCHEMENAME:
				GetDlgItemText(hDlg,IDC_SCHEMENAME,curCS.name,255);
				break;
			case IDC_CURCOLOR:
				{
					HWND lv=GetDlgItem(hDlg,IDC_COLORLIST);
					GetDlgItemText(hDlg,IDC_CURCOLOR,row,255);
					unsigned int color=0;
					for (int i=0;row[i];i++)
					{
						if (row[i]>='0' && row[i]<='9')
						{
							color<<=4;
							color|=row[i]-'0';
						}
						if (row[i]>='a' && row[i]<='f')
						{
							color<<=4;
							color|=row[i]+10-'a';
						}
						if (row[i]>='A' && row[i]<='F')
						{
							color<<=4;
							color|=row[i]+10-'A';
						}
					}
					curCS.colors[curSel]&=0xff;
					curCS.colors[curSel]|=color<<8;
					ListView_RedrawItems(lv,curSel,curSel);
				}
				break;
			case IDC_CURALPHA:
				{
					HWND lv=GetDlgItem(hDlg,IDC_COLORLIST);
					curCS.colors[curSel]&=~0xff;
					curCS.colors[curSel]|=GetDlgItemInt(hDlg,IDC_CURALPHA,NULL,FALSE)&0xff;
					ListView_RedrawItems(lv,curSel,curSel);
				}
			}
			break;
		}
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
		case IDOK:
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
		}
		break;
	}
	return (INT_PTR)FALSE;
}