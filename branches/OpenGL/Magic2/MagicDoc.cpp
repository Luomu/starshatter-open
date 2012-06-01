/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         MagicDoc.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Implementation of the MagicDoc class
*/

#include "stdafx.h"
#include "Magic.h"

#include "MagicDoc.h"
#include "ModelFileMAG.h"
#include "ModelFileOBJ.h"
#include "ModelFile3DS.h"
#include "Selection.h"
#include "Selector.h"
#include "Editor.h"
#include "Command.h"

#include "Bitmap.h"
#include "Color.h"
#include "D3DXImage.h"
#include "Geometry.h"
#include "Pcx.h"
#include "Polygon.h"
#include "Solid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// +--------------------------------------------------------------------+

IMPLEMENT_DYNCREATE(MagicDoc, CDocument)

BEGIN_MESSAGE_MAP(MagicDoc, CDocument)
	//{{AFX_MSG_MAP(MagicDoc)
	ON_COMMAND(ID_SURFACE_OPTIMIZE, OnSurfaceOptimize)
	ON_COMMAND(ID_SURFACE_EXPLODE, OnSurfaceExplode)
	ON_UPDATE_COMMAND_UI(ID_SURFACE_OPTIMIZE, OnUpdateSurfaceOptimize)
	ON_UPDATE_COMMAND_UI(ID_SURFACE_EXPLODE, OnUpdateSurfaceExplode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// +--------------------------------------------------------------------+

MagicDoc::MagicDoc()
  : solid(0), selection(0)
{
   solid     = new Solid;
   selection = new Selection;
   selector  = new Selector(selection);
   editor    = new Editor(this);
}

MagicDoc::~MagicDoc()
{
   if (editor)    delete editor;
   if (selector)  delete selector;
   if (selection) delete selection;
   if (solid)     delete solid;
}

// +--------------------------------------------------------------------+

void
MagicDoc::InitCommandStack()
{
   nundo = 0;
   commands.destroy();
}

void
MagicDoc::Exec(Command* command)
{
   int nredo = commands.size() - nundo;
   
   while (nredo) {
      delete commands.removeIndex(commands.size()-1);
      nredo--;
   }

   if (nundo < 100) {
      nundo++;
   }
   else {
      delete commands.removeIndex(0);
   }

   command->Do();
   commands.append(command);
}

int
MagicDoc::NumUndo() const
{
   return nundo;
}

int
MagicDoc::NumRedo() const
{
   return commands.size() - nundo;
}

const char*
MagicDoc::GetUndoName() const
{
   if (nundo > 0 && nundo <= commands.size())
      return commands[nundo-1]->Name();
   else
      return "";
}

const char*
MagicDoc::GetRedoName() const
{
   if (nundo >= 0 && nundo < commands.size())
      return commands[nundo]->Name();
   else
      return "";
}

void
MagicDoc::Undo()
{
   if (nundo > 0 && nundo <= commands.size())
      commands[--nundo]->Undo();
}

void
MagicDoc::Redo()
{
   if (nundo >= 0 && nundo < commands.size())
      commands[nundo++]->Do();
}

// +--------------------------------------------------------------------+

BOOL MagicDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

   InitCommandStack();

   if (solid) delete solid;
   solid = new Solid;

   if (selection)
      selection->Clear();

	return TRUE;
}

// +--------------------------------------------------------------------+

void MagicDoc::Serialize(CArchive& ar)
{
}

// +--------------------------------------------------------------------+

#ifdef _DEBUG
void MagicDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void MagicDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// +--------------------------------------------------------------------+

BOOL MagicDoc::OnSaveDocument(LPCTSTR path_name) 
{
   SetModifiedFlag(FALSE);

   ModelFileMAG mod_file(path_name);
   mod_file.Save(solid->GetModel());

   SetModifiedFlag(FALSE);
   UpdateAllViews(NULL);
   return TRUE;
}

BOOL MagicDoc::OnOpenDocument(LPCTSTR path_name) 
{
   FILE* fp = fopen(path_name, "rb");
   if (!fp) {
      ::MessageBox(0, "Open Failed:  could not open file", "ERROR", MB_OK);
      return FALSE;
   }

   int version = 1;
   char file_id[5];
   fread(file_id, 4, 1, fp);
   file_id[4] = '\0';
   fclose(fp);

   if (strncmp(file_id, "MAG", 3)) {
      ::MessageBox(0, "Open Failed:  Invalid file type", "ERROR", MB_OK);
      return FALSE;
   }

   switch (file_id[3]) {
   case '6':   version = 6;   break;
   case '5':   version = 5;   break;
   default:    version = 0;   break;
   }

   if (version < 5 || version > 6) {
      ::MessageBox(0, "Open Failed:  Unsupported version", "ERROR", MB_OK);
      return FALSE;
   }

   DeleteContents();

   ModelFileMAG mod_file(path_name);
   solid->Load(&mod_file);
   solid->CreateShadows();

   SetModifiedFlag(FALSE);
   UpdateAllViews(NULL);
   return TRUE;
}

bool
MagicDoc::ImportFile(LPCTSTR path_name)
{
   if (strstr(path_name, ".obj") || strstr(path_name, ".OBJ")) {
      ModelFileOBJ obj_file(path_name);

      if (solid->GetModel()) {
         Solid* s = new Solid;

         if (s->Load(&obj_file)) {
            // todo: insert command here
            Model* orig = solid->GetModel();
            Model* imported = s->GetModel();

            orig->GetMaterials().append(imported->GetMaterials());
            orig->GetSurfaces().append(imported->GetSurfaces());
            orig->OptimizeMaterials();

            imported->GetMaterials().clear();
            imported->GetSurfaces().clear();

            SetModifiedFlag(FALSE);
            UpdateAllViews(NULL);
            delete s;
            return true;
         }

         delete s;
      }
      else {
         if (solid->Load(&obj_file)) {
            SetModifiedFlag(FALSE);
            UpdateAllViews(NULL);
            return true;
         }
      }

      return false;
   }

   if (strstr(path_name, ".3ds") || strstr(path_name, ".3DS")) {
      ModelFile3DS model_file(path_name);

      if (solid->GetModel()) {
         Solid* s = new Solid;

         if (s->Load(&model_file)) {
            // todo: insert command here
            Model* orig = solid->GetModel();
            Model* imported = s->GetModel();

            orig->GetMaterials().append(imported->GetMaterials());
            orig->GetSurfaces().append(imported->GetSurfaces());
            orig->OptimizeMaterials();

            imported->GetMaterials().clear();
            imported->GetSurfaces().clear();

            SetModifiedFlag(FALSE);
            UpdateAllViews(NULL);
            delete s;
            return true;
         }

         delete s;
      }
      else {
         if (solid->Load(&model_file)) {
            SetModifiedFlag(FALSE);
            UpdateAllViews(NULL);
            return true;
         }
      }

      return false;
   }

   FILE* fp = fopen(path_name, "rb");
   if (!fp) {
      ::MessageBox(0, "Import Failed:  could not open file", "ERROR", MB_OK);
      return false;
   }

   int version = 1;
   char file_id[5];
   fread(file_id, 4, 1, fp);
   file_id[4] = '\0';
   fclose(fp);

   if (strncmp(file_id, "MAG", 3)) {
      ::MessageBox(0, "Open Failed:  Invalid file type", "ERROR", MB_OK);
      return false;
   }

   switch (file_id[3]) {
   case '6':   version = 6;   break;
   case '5':   version = 5;   break;
   default:    version = 0;   break;
   }

   if (version < 5 || version > 6) {
      ::MessageBox(0, "Open Failed:  Unsupported version", "ERROR", MB_OK);
      return false;
   }

   ModelFileMAG mag_file(path_name);

   if (solid->GetModel()) {
      Solid* s = new Solid;
      if (s->Load(&mag_file)) {
         // todo: insert command here
         Model* orig = solid->GetModel();
         Model* imported = s->GetModel();

         orig->GetMaterials().append(imported->GetMaterials());
         orig->GetSurfaces().append(imported->GetSurfaces());
         orig->OptimizeMaterials();

         imported->GetMaterials().clear();
         imported->GetSurfaces().clear();

         SetModifiedFlag(FALSE);
         UpdateAllViews(NULL);
         delete s;
         return true;
      }

      delete s;
   }
   else {
      InitCommandStack();

      if (solid->Load(&mag_file)) {
         SetModifiedFlag(FALSE);
         UpdateAllViews(NULL);
         return true;
      }
   }

   return false;
}

bool
MagicDoc::ExportFile(LPCTSTR path_name)
{
   if (!solid->GetModel())
      return false;

   if (strstr(path_name, ".obj") || strstr(path_name, ".OBJ")) {
      ModelFileOBJ obj_file(path_name);
      obj_file.Save(solid->GetModel());
      return true;
   }

   if (strstr(path_name, ".3ds") || strstr(path_name, ".3DS")) {
      return false;
   }

   if (strstr(path_name, ".mag") || strstr(path_name, ".MAG")) {
      ModelFileMAG mod_file(path_name);
      mod_file.Save(solid->GetModel());
      return true;
   }

   return false;
}

// +--------------------------------------------------------------------+

int LoadBuffer(const char* filename, BYTE*& buf, bool null_terminate)
{
   buf = 0;

   FILE* f = ::fopen(filename, "rb");

   if (f) {
      ::fseek(f, 0, SEEK_END);
      int len = ftell(f);
      ::fseek(f, 0, SEEK_SET);

      if (null_terminate) {
         buf = new BYTE[len+1];
         if (buf)
            buf[len] = 0;
      }

      else {
         buf = new BYTE[len];
      }

      if (buf)
         ::fread(buf, len, 1, f);

      ::fclose(f);

      return len;
   }

   return 0;
}

// +--------------------------------------------------------------------+

void MagicDoc::DeleteContents() 
{
	CDocument::DeleteContents();
   InitCommandStack();

   if (solid) {
      delete solid;
      solid = new Solid;
   }

   if (selection)
      selection->Clear();
}

// +--------------------------------------------------------------------+

int LoadTexture(const char* fname, Bitmap*& bitmap, int type)
{
   int   result = 0;
   
   if (!fname || !*fname)
      return result;

   bitmap = Bitmap::CheckCache(fname);

   if (!bitmap) {
      bool pcx_file = strstr(fname, ".pcx") || strstr(fname, ".PCX");

      // handle PCX formats:
      if (pcx_file) {
         PcxImage pcx;

         if (pcx.Load((char*) fname) == PCX_OK) {
            bitmap = new Bitmap;

            // 32-bit image
            if (pcx.himap) {
               bitmap->CopyHighColorImage(pcx.width, pcx.height, pcx.himap);
            }

            // 8-bit image, check for 32-bit image as well
            else if (pcx.bitmap) {
               bitmap->CopyImage(pcx.width, pcx.height, pcx.bitmap);

               char tmp[256];
               int  len = strlen(fname);
               bool found = false;

               ZeroMemory(tmp, sizeof(tmp));

               for (int i = 0; i < len && !found; i++) {
                  if (strstr(fname + i, ".pcx") == (fname+i)) {
                     found = true;
                  }
                  else {
                     tmp[i] = fname[i];
                  }
               }

               if (found) {
                  strcat_s(tmp, "+.pcx");
                  if (pcx.Load(tmp) == PCX_OK && pcx.himap != 0) {
                     bitmap->CopyHighColorImage(pcx.width, pcx.height, pcx.himap);
                  }
               }
            }
         }
      }

      // for all other formats, use D3DX:
      else {
         D3DXImage d3dx;
         if (d3dx.Load((char*) fname)) {
            bitmap = new Bitmap;
            bitmap->CopyHighColorImage(d3dx.width, d3dx.height, d3dx.image);
         }
      }

      if (bitmap) {
         LoadAlpha(fname, *bitmap, type);

         bitmap->SetFilename(fname);
         bitmap->SetType(type);
         bitmap->MakeTexture();

         Bitmap::AddToCache(bitmap);
      }
   }

   return result;
}

int LoadAlpha(const char* name, Bitmap& bitmap, int type)
{
   PcxImage    pcx;
   D3DXImage   d3dx;
   bool        pcx_file = strstr(name, ".pcx") || strstr(name, ".PCX");
   bool        bmp_file = strstr(name, ".bmp") || strstr(name, ".BMP");
   bool        jpg_file = strstr(name, ".jpg") || strstr(name, ".JPG");
   bool        png_file = strstr(name, ".png") || strstr(name, ".PNG");
   bool        tga_file = strstr(name, ".tga") || strstr(name, ".TGA");

   // check for an associated alpha-only (grayscale) bitmap:
   char filename[256];
   strcpy_s(filename, name);

   char* dot = strrchr(filename, '.');
   if (dot && pcx_file)
      strcpy(dot, "@.pcx");
   else if (dot && bmp_file)
      strcpy(dot, "@.bmp");
   else if (dot && jpg_file)
      strcpy(dot, "@.jpg");
   else if (dot && png_file)
      strcpy(dot, "@.png");
   else if (dot && tga_file)
      strcpy(dot, "@.tga");
   else
      return 0;

   // first try to load from current directory:
   bool loaded = false;
   
   if (pcx_file)
      loaded = pcx.Load(filename) == PCX_OK;

   else
      loaded = d3dx.Load(filename);

   // now copy the alpha values into the bitmap:
   if (loaded) {
      if (pcx_file && pcx.bitmap) {
         bitmap.CopyAlphaImage(pcx.width, pcx.height, pcx.bitmap);
      }
      else if (pcx_file && pcx.himap) {
         bitmap.CopyAlphaRedChannel(pcx.width, pcx.height, pcx.himap);
      }
      else if (d3dx.image) {
         bitmap.CopyAlphaRedChannel(d3dx.width, d3dx.height, d3dx.image);
      }
   }

   return 0;
}


void MagicDoc::OnSurfaceOptimize() 
{
   if (solid && solid->GetModel()) {
      solid->GetModel()->OptimizeMesh();
      solid->InvalidateSurfaceData();
      solid->InvalidateSegmentData();
   }
}

void MagicDoc::OnUpdateSurfaceOptimize(CCmdUI* pCmdUI) 
{
   pCmdUI->Enable(solid && solid->GetModel());
}

void MagicDoc::OnSurfaceExplode() 
{
   if (solid && solid->GetModel()) {
      solid->GetModel()->ExplodeMesh();
      solid->InvalidateSurfaceData();
      solid->InvalidateSegmentData();
   }
}

void MagicDoc::OnUpdateSurfaceExplode(CCmdUI* pCmdUI) 
{
   pCmdUI->Enable(solid && solid->GetModel());
}
