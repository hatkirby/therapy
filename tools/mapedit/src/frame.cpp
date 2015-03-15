#include "frame.h"
#include "widget.h"
#include "tile_widget.h"
#include <wx/splitter.h>
#include "panel.h"

enum {
  MENU_VIEW_ZOOM_IN,
  MENU_VIEW_ZOOM_OUT,
  MENU_FILE_NEW,
  MENU_FILE_OPEN,
  MENU_FILE_SAVE,
  MENU_FILE_CLOSE
};

wxBEGIN_EVENT_TABLE(MapeditFrame, wxFrame)
  EVT_MENU(wxID_EXIT, MapeditFrame::OnQuit)
  EVT_MENU(MENU_VIEW_ZOOM_IN, MapeditFrame::ZoomIn)
  EVT_MENU(MENU_VIEW_ZOOM_OUT, MapeditFrame::ZoomOut)
  EVT_MENU(MENU_FILE_NEW, MapeditFrame::OnNew)
  EVT_MENU(MENU_FILE_OPEN, MapeditFrame::OnOpen)
  EVT_MENU(MENU_FILE_SAVE, MapeditFrame::OnSave)
  EVT_MENU(MENU_FILE_CLOSE, MapeditFrame::OnClose)
  EVT_CLOSE(MapeditFrame::OnExit)
wxEND_EVENT_TABLE()

MapeditFrame::MapeditFrame(Map map, std::string filename) : wxFrame(NULL, wxID_ANY, "Map Editor", wxDefaultPosition, wxSize(GAME_WIDTH*3, GAME_HEIGHT*2)), map(map), filename(filename)
{
  wxMenu* menuFile = new wxMenu;
  menuFile->Append(MENU_FILE_NEW, "New\tCtrl-N");
  menuFile->Append(MENU_FILE_OPEN, "Open\tCtrl-O");
  menuFile->Append(MENU_FILE_SAVE, "Save\tCtrl-S");
  menuFile->Append(MENU_FILE_CLOSE, "Close\tCtrl-W");
  menuFile->Append(wxID_EXIT);
  
  wxMenu* menuView = new wxMenu;
  menuView->Append(MENU_VIEW_ZOOM_IN, "Zoom In\tCtrl-+");
  menuView->Append(MENU_VIEW_ZOOM_OUT, "Zoom Out\tCtrl--");

  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuView, "&View");
  
  SetMenuBar(menuBar);
  
  wxBoxSizer* sizermain = new wxBoxSizer(wxVERTICAL);
  wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY);
  splitter->SetSashGravity(0.0);
  splitter->SetMinimumPaneSize(50);
  sizermain->Add(splitter, 1, wxEXPAND, 0);
  
  wxPanel* tileEditorPanel = new wxPanel(splitter, wxID_ANY);
  tileEditor = new TileWidget(tileEditorPanel, wxID_ANY, 6, 6, wxPoint(0,0));
  wxBoxSizer* tileSizer = new wxBoxSizer(wxVERTICAL);
  tileSizer->Add(tileEditor, 1, wxEXPAND, 0);
  tileEditorPanel->SetSizer(tileSizer);
  
  wxPanel* mapEditorPanel = new wxPanel(splitter, wxID_ANY);
  mapEditor = new MapeditWidget(mapEditorPanel, wxID_ANY, &this->map, tileEditor, wxPoint(0,0));
  wxBoxSizer* mapSizer = new wxBoxSizer(wxVERTICAL);
  mapSizer->Add(mapEditor, 1, wxEXPAND, 0);
  mapEditorPanel->SetSizer(mapSizer);
  
  splitter->SplitVertically(tileEditorPanel, mapEditorPanel);
  
  this->SetSizer(sizermain);
  sizermain->SetSizeHints(this);
}

void MapeditFrame::OnExit(wxCloseEvent& event)
{
  if (event.CanVeto() && map.hasUnsavedChanges())
  {
    switch (wxMessageBox("Current map has unsaved changes. Save before closing?", "Please confirm", wxICON_QUESTION|wxYES_NO|wxCANCEL, this))
    {
      case wxYES:
        if (filename == "")
        {
          wxFileDialog saveFileDialog(this, "Save map", "", "", "XML files (*.xml)|*.xml", wxFD_SAVE);
          if (saveFileDialog.ShowModal() == wxID_CANCEL)
          {
            return;
          }
    
          filename = saveFileDialog.GetPath().ToStdString();
        }
  
        map.save(filename);
        break;
        
      case wxCANCEL:
        event.Veto(true);
        return;
    }
  }
  
  event.Skip();
}

MapeditWidget* MapeditFrame::GetMapEditor()
{
  return mapEditor;
}

void MapeditFrame::ZoomIn(wxCommandEvent& event)
{
  mapEditor->ZoomIn();
}

void MapeditFrame::ZoomOut(wxCommandEvent& event)
{
  mapEditor->ZoomOut();
}

void MapeditFrame::OnNew(wxCommandEvent& event)
{
  MapeditFrame* frame = new MapeditFrame();
  frame->Show(true);
}

void MapeditFrame::OnOpen(wxCommandEvent& event)
{
  wxFileDialog openFileDialog(this, "Open map", "", "", "XML files (*.xml)|*.xml", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  if (openFileDialog.ShowModal() == wxID_CANCEL)
  {
    return;
  }
  
  std::string filename = openFileDialog.GetPath().ToStdString();
  MapeditFrame* frame = new MapeditFrame(Map(filename), filename);
  frame->Show(true);
}

void MapeditFrame::OnSave(wxCommandEvent& event)
{
  if (filename == "")
  {
    wxFileDialog saveFileDialog(this, "Save map", "", "", "XML files (*.xml)|*.xml", wxFD_SAVE);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
    {
      return;
    }
    
    filename = saveFileDialog.GetPath().ToStdString();
  }
  
  map.save(filename);
}

void MapeditFrame::OnClose(wxCommandEvent& event)
{
  Close(false);
}

void MapeditFrame::OnQuit(wxCommandEvent& event)
{
  // TODO
}
