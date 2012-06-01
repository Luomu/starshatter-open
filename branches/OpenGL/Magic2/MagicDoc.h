/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         MagicDoc.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Interface of the MagicDoc class
*/


#if !defined(AFX_MAGICDOC_H__8B8D63A3_30F9_4023_BFA8_DB79891487C2__INCLUDED_)
#define AFX_MAGICDOC_H__8B8D63A3_30F9_4023_BFA8_DB79891487C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// +--------------------------------------------------------------------+

#include "List.h"

// +--------------------------------------------------------------------+

class Bitmap;
class Command;
class Editor;
class Solid;
class Model;
class Surface;
class Segment;
class Selection;
class Selector;

// +--------------------------------------------------------------------+

class MagicDoc : public CDocument
{
protected: // create from serialization only
	MagicDoc();
	DECLARE_DYNCREATE(MagicDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MagicDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

   void        InitCommandStack();
   void        Exec(Command* command);
   void        Undo();
   void        Redo();
   int         NumUndo()      const;
   int         NumRedo()      const;
   const char* GetUndoName()  const;
   const char* GetRedoName()  const;

// Implementation
public:
	virtual ~MagicDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

   Solid*      GetSolid()           { return solid;      }
   Selection*  GetSelection()       { return selection;  }
   Selector*   GetSelector()        { return selector;   }
   Editor*     GetEditor()          { return editor;     }

   bool  ImportFile(LPCTSTR path_name);
   bool  ExportFile(LPCTSTR path_name);

protected:
   Solid*         solid;
   Selection*     selection;
   Selector*      selector;
   Editor*        editor;

   List<Command>  commands;
   int            nundo;

// Generated message map functions
protected:
	//{{AFX_MSG(MagicDoc)
	afx_msg void OnSurfaceOptimize();
	afx_msg void OnSurfaceExplode();
	afx_msg void OnUpdateSurfaceOptimize(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSurfaceExplode(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// +--------------------------------------------------------------------+

int LoadBuffer(const char* filename, BYTE*& buf, bool null_terminate=false);
int LoadTexture(const char* name, Bitmap*& bmp, int type=0);
int LoadAlpha(const char* name, Bitmap& bitmap, int type=0);

// +--------------------------------------------------------------------+

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAGICDOC_H__8B8D63A3_30F9_4023_BFA8_DB79891487C2__INCLUDED_)
