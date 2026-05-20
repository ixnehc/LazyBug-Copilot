#pragma once

#include "GuiLib.h"

#include <string>
#include <vector>

extern int ListBox_Find(CListBox *lb,const char *name,BOOL bCaseSensitive);

extern void ListBox_UpdateItems(CListBox *lb,std::vector<std::string>&items,BOOL bCaseSensitive);

extern BOOL ListBox_CheckDupe(CListBox *lb,DWORD iItem,BOOL bCaseSensitive);

extern BOOL ListBox_IsSelected(CListBox *lb,DWORD iItem);


extern const char *ListBox_GetSelString(CListBox *lb);
extern const char *ListBox_GetString(CListBox *lb,DWORD iItem);

extern BOOL ListBox_SetItemText(CListBox *lb,DWORD iItem,const char *str);


extern int ComboBox_Find(CComboBox *cb,const char *name,BOOL bCaseSensitive);
extern const char *ComboBox_GetSelString(CComboBox *cb);
extern int ComboBox_GetListSel(CComboBox *cb);//Get the select of the list box

extern BOOL InputString(const char *prompt,std::string &s);
