// AccelEditor.cpp : implementation file
//

#include "../stdafx.h"
#include "../resource.h"
#include "AccelEditor.h"
#include "../CmdAccelOb.h"
#include "../VBA.h"

/////////////////////////////////////////////////////////////////////////////
// AccelEditor dialog

AccelEditor::AccelEditor(CWnd *pParent, CMenu *pMenu, CAcceleratorManager *pExtMgr)
	: ResizeDlg(AccelEditor::IDD, pParent), m_pMenuSrc(pMenu), m_pExtMgr(pExtMgr)
{
	//{{AFX_DATA_INIT(AccelEditor)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BOOL AccelEditor::IsModified() const
{
	return m_modified;
}

const CAcceleratorManager &AccelEditor::GetResultMangager() const
{
	return m_result;
}

void AccelEditor::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AccelEditor)
	DDX_Control(pDX, IDC_CURRENTS, m_currents);
	DDX_Control(pDX, IDC_ALREADY_AFFECTED, m_alreadyAffected);
	DDX_Control(pDX, IDC_COMMANDS, m_commands);
	DDX_Control(pDX, IDC_EDIT_KEY, m_key);
	DDX_Control(pDX, IDC_ACCELEDIT_AUTOTIMEOUT, m_timeout);
	DDX_Control(pDX, IDC_ACCELEDIT_PROGRESSBAR, m_progress);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(AccelEditor, CDialog)
//{{AFX_MSG_MAP(AccelEditor)
ON_BN_CLICKED(ID_OK, &AccelEditor::OnOk)
ON_BN_CLICKED(ID_CANCEL, &AccelEditor::OnCancel)
ON_BN_CLICKED(IDC_ACCELEDIT_APPLY, &AccelEditor::OnApply)
ON_BN_CLICKED(IDC_RESET, &AccelEditor::OnReset)
ON_BN_CLICKED(IDC_ASSIGN, &AccelEditor::OnAssign)
ON_BN_CLICKED(IDC_REMOVE, &AccelEditor::OnRemove)
ON_BN_CLICKED(IDC_ACCELEDIT_REPLACE, &AccelEditor::OnReplace)
ON_CONTROL(EN_CHANGE, IDC_EDIT_KEY, &AccelEditor::OnKeyboardEditChange)
ON_CONTROL(EN_KILLFOCUS, IDC_EDIT_KEY, &AccelEditor::OnKeyboardEditKillfocus)
ON_CONTROL(EN_SETFOCUS, IDC_ACCELEDIT_AUTOTIMEOUT, &AccelEditor::OnTimeoutEditSetfocus)
ON_CONTROL(EN_KILLFOCUS, IDC_ACCELEDIT_AUTOTIMEOUT, &AccelEditor::OnTimeoutEditKillfocus)
ON_NOTIFY(TVN_SELCHANGED, IDC_COMMANDS, &AccelEditor::OnTvnSelchangedCommands)
//ON_NOTIFY(LVN_ITEMCHANGED, IDC_CURRENTS, &AccelEditor::OnListItemChanged)
ON_NOTIFY(NM_DBLCLK, IDC_CURRENTS, &AccelEditor::OnListDblClick)
ON_NOTIFY(NM_CLICK, IDC_CURRENTS, &AccelEditor::OnListClick)
ON_WM_TIMER()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AccelEditor message handlers

BOOL AccelEditor::OnInitDialog()
{
	CDialog::OnInitDialog();

	DIALOG_SIZER_START(sz)
	DIALOG_SIZER_ENTRY(ID_OK, DS_MoveX)
	DIALOG_SIZER_ENTRY(ID_CANCEL, DS_MoveX)
	DIALOG_SIZER_ENTRY(IDC_ACCELEDIT_APPLY, DS_MoveX)
	DIALOG_SIZER_ENTRY(IDC_ASSIGN, DS_MoveX)
	DIALOG_SIZER_ENTRY(IDC_REMOVE, DS_MoveX)
	DIALOG_SIZER_ENTRY(IDC_ACCELEDIT_REPLACE, DS_MoveX)
	DIALOG_SIZER_ENTRY(IDC_COMMANDS, DS_SizeX | DS_SizeY)
	DIALOG_SIZER_ENTRY(IDC_CURRENTS, DS_MoveX | DS_SizeY)
	DIALOG_SIZER_ENTRY(IDC_EDIT_KEY, DS_MoveX | DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_EDIT_KEY, DS_MoveX | DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_STATIC2, DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_STATIC3, DS_MoveX | DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_STATIC4, DS_MoveX | DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_STATIC5, DS_MoveX | DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_ACCELEDIT_AUTOTIMEOUT, DS_SizeX | DS_MoveY)
	DIALOG_SIZER_END()
	SetData(sz,
	        TRUE,
	        HKEY_CURRENT_USER,
	        "Software\\Emulators\\VisualBoyAdvance\\Viewer\\AccelEditor",
	        NULL);

	if (m_pExtMgr)
		m_result = m_mgr = *m_pExtMgr;

	m_currents.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_currents.InsertColumn(0, "Keys");
	m_currents.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	InitCommands();
	m_autoMode	   = AUTO_REPLACE;
	m_modified	   = FALSE;
	m_timeoutValue = 1000;
	CString timeoutStr;
	timeoutStr.Format("%d", m_timeoutValue);
	m_timeout.SetWindowText(timeoutStr);
	m_progress.SetPos(0);

	GetDlgItem(IDC_ACCELEDIT_APPLY)->EnableWindow(FALSE);
	return TRUE; // return TRUE unless you set the focus to a control
	             // EXCEPTION: OCX Property Pages should return FALSE
}

void AccelEditor::AddCommandsFromTable()
{
	POSITION pos = m_mgr.m_mapAccelString.GetStartPosition();
	while (pos != NULL)
	{
		CString command;
		WORD	wID;
		m_mgr.m_mapAccelString.GetNextAssoc(pos, command, wID);
		int nPos = command.Find('\\');

		if (nPos == 0)  // skip menu commands
		{
			continue;
		}

		HTREEITEM newItem = TVI_ROOT;
#if 0
/*
        while (nPos != -1)
        {
            newItem = m_commands.InsertItem(command.Left(nPos), newItem);
            command.Delete(0, nPos + 1);
            nPos = command.Find('\\');
        }
 */
#endif
		newItem = m_commands.InsertItem(command, newItem);
		m_commands.SetItemData(newItem, wID);
		m_hItems.AddTail(newItem);
	}
}

// recursive calls
void AccelEditor::AddCommandsFromMenu(CMenu *pMenu, HTREEITEM hParent)
{
	UINT nIndexMax = pMenu->GetMenuItemCount();
	for (UINT nIndex = 0; nIndex < nIndexMax; ++nIndex)
	{
		UINT nID = pMenu->GetMenuItemID(nIndex);
		if (nID == 0)
			continue;  // menu separator or invalid cmd - ignore it

		if (nID == (UINT)-1)
		{
			// possibly a submenu
			CMenu *pSubMenu = pMenu->GetSubMenu(nIndex);
			if (pSubMenu != NULL)
			{
				CString tempStr;
				pMenu->GetMenuString(nIndex, tempStr, MF_BYPOSITION);
				tempStr.Remove('&');
				HTREEITEM newItem = m_commands.InsertItem(tempStr, hParent);
				AddCommandsFromMenu(pSubMenu, newItem);
			}
		}
		else
		{
			// normal menu item
			// generate the strings
			CString command;
			pMenu->GetMenuString(nIndex, command, MF_BYPOSITION);
			int nPos = command.ReverseFind('\t');
			if (nPos != -1)
			{
				command.Delete(nPos, command.GetLength() - nPos);
			}
			command.Remove('&');
			HTREEITEM newItem = m_commands.InsertItem(command, hParent);
			m_commands.SetItemData(newItem, nID);
			m_hItems.AddTail(newItem);
		}
	}
}

void AccelEditor::InitCommands()
{
	m_commands.DeleteAllItems();
	m_hItems.RemoveAll();
	m_alreadyAffected.SetWindowText("");

	AddCommandsFromMenu(m_pMenuSrc, TVI_ROOT);
	AddCommandsFromTable();
}

BOOL AccelEditor::PreTranslateMessage(MSG *pMsg)
{
	CWnd *pFocus = GetFocus();
	if (pFocus == &m_currents)
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			switch (pMsg->wParam)
			{
			case VK_ESCAPE:
				m_currents.SetItemState(-1, 0, LVIS_SELECTED);
				CheckListSelections();
				break;
			case VK_RETURN:
			case VK_INSERT:
				// kludge to workaround CKeyboardEdit::PreTranslateMessage()
				break;
			case VK_DELETE:
			case VK_BACK:
				OnRemove();
				break;
			case VK_F6:
			case VK_LEFT:
				m_commands.SetFocus();
				break;
			case VK_RIGHT:
				GetDlgItem(ID_OK)->SetFocus();
			default:
				return ResizeDlg::PreTranslateMessage(pMsg);
			}
			return TRUE;
		}
		else if (pMsg->message == WM_KEYUP)	// kludge to workaround CKeyboardEdit::PreTranslateMessage()
		{
			switch (pMsg->wParam)
			{
			case VK_RETURN:
				OnEdit();
				break;
			case VK_INSERT:
				OnNew();
				break;
			default:
				return ResizeDlg::PreTranslateMessage(pMsg);
			}
			return TRUE;
		}
	}
	else if (pFocus == &m_commands)
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			switch (pMsg->wParam)
			{
			case VK_F6:
				m_currents.SetFocus();
				break;
			case VK_RIGHT:
				if (!m_commands.ItemHasChildren(m_commands.GetSelectedItem()))
				{
					m_currents.SetFocus();
					break;
				}
				// fall through
			default:
				return ResizeDlg::PreTranslateMessage(pMsg);
			}
			return TRUE;
		}
	}

	return ResizeDlg::PreTranslateMessage(pMsg);
}

void AccelEditor::OnOk()
{
	OnApply();
//	OnTimeoutEditKillfocus();
	EndDialog(TRUE);
}

void AccelEditor::OnCancel()
{
//	OnTimeoutEditKillfocus();
//	EndDialog(m_modified);
	EndDialog(FALSE);	// this allows the caller to cancel even if the user has Apply'ed
}

void AccelEditor::OnApply()
{
	m_result   = m_mgr;
	GetDlgItem(IDC_ACCELEDIT_APPLY)->EnableWindow(FALSE);
}

void AccelEditor::OnReset()
{
	m_mgr.Default(); /// FIXME accelerator reset NYI
	systemMessage(
	    0,
	    "The \"Reset All Accelerators\" feature is currently unimplemented.\nYou can achieve the same result by closing VBA, opening up your \"vba.ini\" file, deleting the line that starts with \"keyboard\", then reopening VBA.");
	InitCommands(); // update the listboxes.
}

void AccelEditor::OnAssign()
{
	if (CheckAffected())
		return;

	// get the currently selected group
	HTREEITEM hItem = m_commands.GetSelectedItem();
	if (hItem == NULL)
		return;  // abort

	// Get the object who manage the accels list, associated to the command.
	WORD wIDCommand = LOWORD(m_commands.GetItemData(hItem));

	CCmdAccelOb *pCmdAccel;
	if (m_mgr.m_mapAccelTable.Lookup(wIDCommand, pCmdAccel) != TRUE)
		return;

	WORD wKey;
	bool bCtrl, bAlt, bShift;
	if (!m_key.GetAccelKey(wKey, bCtrl, bAlt, bShift))
		return;  // no valid key, abort

	BYTE cVirt = 0;
	if (bCtrl)
		cVirt |= FCONTROL;
	if (bAlt)
		cVirt |= FALT;
	if (bShift)
		cVirt |= FSHIFT;

	cVirt |= FVIRTKEY;

	// Create the new key...
	CAccelsOb *pAccel = new CAccelsOb(cVirt, wKey, false);
	ASSERT(pAccel != NULL);
	// ...and add in the list.
	pCmdAccel->m_Accels.AddTail(pAccel);

	// Update the listbox.
	CString szBuffer;
	pAccel->GetString(szBuffer);

	int index = m_currents.GetNextItem(-1, LVNI_SELECTED);
	if (index < 0)
		index = 0;
	m_currents.InsertItem(index, szBuffer);
	m_currents.SetItemData(index, reinterpret_cast<DWORD>(pAccel));
	m_currents.SetItemState(-1, 0, LVIS_SELECTED);	// deselect other items first
	m_currents.SetItemState(index, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
	GetDlgItem(IDC_ACCELEDIT_REPLACE)->EnableWindow(TRUE);

	// Reset the key editor.
//	m_key.ResetKey();

	m_modified = TRUE;
	GetDlgItem(IDC_ACCELEDIT_APPLY)->EnableWindow(TRUE);
}

void AccelEditor::OnRemove()
{
	// Some controls
	POSITION selected = m_currents.GetFirstSelectedItemPosition();
	if (selected == NULL)
		return;

	HTREEITEM hItem = m_commands.GetSelectedItem();
	if (hItem == NULL)
		return;

	// Ref to the ID command
	WORD wIDCommand = LOWORD(m_commands.GetItemData(hItem));

	// Run through the accels, and control if it can be deleted.
	CCmdAccelOb *pCmdAccel;
	if (m_mgr.m_mapAccelTable.Lookup(wIDCommand, pCmdAccel) == TRUE)
	{
		POSITION pos = pCmdAccel->m_Accels.GetHeadPosition();
		POSITION PrevPos;
		while (pos != NULL)
		{
			PrevPos = pos;
			CAccelsOb *pAccel = pCmdAccel->m_Accels.GetNext(pos);
			do
			{
				int indexCurrent = m_currents.GetNextSelectedItem(selected);
				CAccelsOb *pAccelCurrent = reinterpret_cast<CAccelsOb *>(m_currents.GetItemData(indexCurrent));
				if (pAccel == pAccelCurrent)
				{
					if (!pAccel->m_bLocked)
					{
						// not locked, so we delete the key
						pCmdAccel->m_Accels.RemoveAt(PrevPos);
						delete pAccel;
						// and update the listboxes/key editor/static text
						m_currents.DeleteItem(indexCurrent);
						m_modified = TRUE;
						break;
					}
					else
					{
						systemMessage(0, "Unable to remove this locked accelerator: ", m_currents.GetItemText(indexCurrent, KEY_COLUMN));
						m_currents.SetItemState(indexCurrent, 0, LVIS_SELECTED); // deselect it
						break;
					}
				}
			}
			while (selected != NULL);

			selected = m_currents.GetFirstSelectedItemPosition();
			if (selected == NULL)	// the normal exit of this function
			{
				m_currents.SetItemState(m_currents.GetNextItem(-1, LVIS_FOCUSED), LVIS_SELECTED, LVIS_SELECTED);
				if (m_currents.GetSelectedCount() == 0)
				{
					GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
					GetDlgItem(IDC_ACCELEDIT_REPLACE)->EnableWindow(FALSE);
				}
				GetDlgItem(IDC_ACCELEDIT_APPLY)->EnableWindow(m_modified);
				return;
			}
		}
		systemMessage(0, "internal error (AccelEditor::Remove : pAccel unavailable)");
		return;
	}
	systemMessage(0, "internal error (AccelEditor::Remove : Lookup failed)");
}

void AccelEditor::OnReplace()
{
	if (CheckAffected())
		return;
	OnRemove();
	OnAssign();
}

void AccelEditor::OnNew()
{
	m_autoMode = AUTO_NEW;
	m_key.SetFocus();
}

void AccelEditor::OnEdit()
{
	m_autoMode = AUTO_REPLACE;
	m_key.SetFocus();
}

BOOL AccelEditor::CheckAffected()
{
	m_alreadyAffected.SetWindowText("");

	WORD wKey;
	bool bCtrl, bAlt, bShift;
	if (!m_key.GetAccelKey(wKey, bCtrl, bAlt, bShift))
		return TRUE;  // no valid key, abort

	POSITION posItem = m_hItems.GetHeadPosition();
	while (posItem != NULL)
	{
		HTREEITEM hItem		  = m_hItems.GetNext(posItem);
		WORD	  wIDCommand2 = LOWORD(m_commands.GetItemData(hItem));

		CCmdAccelOb *pCmdAccel;
		m_mgr.m_mapAccelTable.Lookup(wIDCommand2, pCmdAccel);

		POSITION pos = pCmdAccel->m_Accels.GetHeadPosition();
		while (pos != NULL)
		{
			CAccelsOb *pAccel = pCmdAccel->m_Accels.GetNext(pos);
			if (pAccel->IsEqual(wKey, bCtrl, bAlt, bShift))
			{
				// the key is already affected (in the same or other command)
				// (the parts that were commented out allow for a one-to-many mapping,
				//  which is only disabled because the MFC stuff that automagically activates the commands
				//  doesn't seem to be capable of activating more than one command per accelerator...)
				m_alreadyAffected.SetWindowText(pCmdAccel->m_szCommand);
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL AccelEditor::CheckJammed()
{
	WORD jam;
	if (m_key.GetJamKey(jam))
	{
		// these go first, or the timer would be set again
		m_key.ResetKey();
		m_key.SetWindowText("Interrupted");
		if (m_currents.IsWindowEnabled())
			m_currents.SetFocus();
		else
			m_commands.SetFocus();
		return TRUE;
	}
	return FALSE;
}

BOOL AccelEditor::CheckListSelections()
{
	BOOL result = m_currents.GetFirstSelectedItemPosition() ? TRUE : FALSE;

	GetDlgItem(IDC_REMOVE)->EnableWindow(result);
	GetDlgItem(IDC_ACCELEDIT_REPLACE)->EnableWindow(result);

	return result;
}

void AccelEditor::OnTvnSelchangedCommands(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	// TODO: Add your control notification handler code here
	// Check if some commands exist.
	HTREEITEM hItem = m_commands.GetSelectedItem();
	if (hItem == NULL)
		return;

	m_currents.DeleteAllItems();

	WORD wIDCommand = LOWORD(m_commands.GetItemData(hItem));
	CCmdAccelOb *pCmdAccel;
	if (m_mgr.m_mapAccelTable.Lookup(wIDCommand, pCmdAccel))
	{
		CAccelsOb *pAccel;
		CString	   szBuffer;
		POSITION   pos = pCmdAccel->m_Accels.GetHeadPosition();

		// Add the keys to the 'currents keys' listbox.
		while (pos != NULL)
		{
			pAccel = pCmdAccel->m_Accels.GetNext(pos);
			pAccel->GetString(szBuffer);
			int index = m_currents.InsertItem(m_currents.GetItemCount(), szBuffer);
			// and a pointer to the accel object.
			m_currents.SetItemData(index, (DWORD)pAccel);
		}

		m_currents.SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		GetDlgItem(IDC_ASSIGN)->EnableWindow(TRUE);
		m_currents.EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_ASSIGN)->EnableWindow(FALSE);
		m_currents.EnableWindow(FALSE);
	}

	// Init the key editor
//  m_pKey->ResetKey();
//	m_alreadyAffected.SetWindowText("");

	CheckListSelections();

	*pResult = 0;
}

/*
void AccelEditor::OnListItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = reinterpret_cast<NMLISTVIEW *>(pNMHDR);
	if (pNMListView->uChanged == LVIF_STATE)
	{
		if ((pNMListView->uOldState & LVIS_SELECTED) && !(pNMListView->uNewState & LVIS_SELECTED))
		{
		}
	}

	*pResult = 0;
}
*/

void AccelEditor::OnListClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	CheckListSelections();
	*pResult = 0;
}

void AccelEditor::OnListDblClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_currents.GetFirstSelectedItemPosition())
		OnEdit();
	else
		OnNew();
	*pResult = 0;
}

void AccelEditor::OnKeyboardEditChange()
{
	if (!m_key.IsDefined())
		return;

//	if (CheckJammed())
//		return;

	OnKeyboardEditKillfocus();
	CheckAffected();
	if (m_timeoutValue == 0)
		return;

	m_progress.SetRange32(0, m_timeoutValue);
	SetTimer(1, 50, NULL);
}

void AccelEditor::OnKeyboardEditKillfocus()
{
	KillTimer(1);
	m_timer = 0;
	m_progress.SetPos(0);
	m_progress.SetBarColor(RGB(128, 0, 255));
}

void AccelEditor::OnTimeoutEditSetfocus()
{
	m_timeout.PostMessage(EM_SETSEL, 0, -1);
}

void AccelEditor::OnTimeoutEditKillfocus()
{
	CString str;
	m_timeout.GetWindowText(str);
	m_timeoutValue = atoi(str);
	m_autoMode	   = AUTO_REPLACE;
}

void AccelEditor::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		m_timer += 50;
		if (m_timer >= m_timeoutValue)
		{
			m_progress.SetPos(m_timeoutValue);
			m_progress.SetBarColor(RGB(255, 255, 0));
			if (m_autoMode == AUTO_NEW)
			{
				OnAssign();
			}
			else
			{
				OnReplace();
			}
			if (m_currents.IsWindowEnabled())
				m_currents.SetFocus();
			else
				m_commands.SetFocus();
			return;
		}
		UINT green = (m_timer * 255 / m_timeoutValue);
		m_progress.SetBarColor(RGB(128 + green / 2, green, 255 - green));
		m_progress.SetPos(m_timer);
	}
}

