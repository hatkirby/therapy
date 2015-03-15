#include "frame.h"
#include "widget.h"
#include "tile_widget.h"
#include <wx/splitter.h>
#include "panel.h"
#include <list>

static std::list<wxWindow*> openWindows;

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
  
  // Layout 1: Splitter between map tree and layout 2
  // Layout 2: Non-splitter between layout 3 and tile chooser
  // Layout 3: Splitter between map editor and properties editor
  
  wxSplitterWindow* layout3 = new wxSplitterWindow(this, wxID_ANY);
  layout3->SetSashGravity(1.0);
  layout3->SetMinimumPaneSize(20);
  
  tileEditor = new TileWidget(this, wxID_ANY, 6, 6, wxPoint(0,0), wxSize(TILE_WIDTH*6*7,TILE_HEIGHT*10*6));
  mapEditor = new MapeditWidget(layout3, wxID_ANY, &this->map, tileEditor, wxPoint(0,0), wxSize(GAME_WIDTH*2, GAME_HEIGHT*2));
  
  wxPanel* propertyEditor = new wxPanel(layout3, wxID_ANY);
  titleBox = new wxTextCtrl(propertyEditor, wxID_ANY, map.getTitle());
  titleBox->Bind(wxEVT_TEXT, &MapeditFrame::OnTitleChange, this);
  
  wxStaticText* titleLabel = new wxStaticText(propertyEditor, wxID_ANY, "Title:");
  
  wxFlexGridSizer* propertySizer = new wxFlexGridSizer(1, 2, 9, 25);
  propertySizer->Add(titleLabel);
  propertySizer->Add(titleBox, 1, wxEXPAND);
  propertyEditor->SetSizer(propertySizer);
  propertySizer->SetSizeHints(propertyEditor);
  
  layout3->SplitHorizontally(mapEditor, propertyEditor);
  
  wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
  sizer2->Add(layout3, 1, wxEXPAND, 0);
  sizer2->Add(tileEditor, 0, wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL | wxLEFT, 2);
  this->SetSizer(sizer2);
  sizer2->SetSizeHints(this);
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
  
  *closer = nullptr;
  
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
  NewMap();
}

void MapeditFrame::OnOpen(wxCommandEvent& event)
{
  wxFileDialog openFileDialog(this, "Open map", "", "", "XML files (*.xml)|*.xml", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  if (openFileDialog.ShowModal() == wxID_CANCEL)
  {
    return;
  }
  
  OpenMap(openFileDialog.GetPath().c_str());
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
  for (auto window : openWindows)
  {
    if (window != nullptr)
    {
      window->Close(false);
    }
  }
}

void MapeditFrame::OnTitleChange(wxCommandEvent& event)
{
  map.setTitle(titleBox->GetLineText(0).ToStdString());
}

void MapeditFrame::NewMap()
{
  LaunchWindow(Map(), "");
}

void MapeditFrame::OpenMap(const char* filename)
{
  LaunchWindow(Map(filename), filename);
}

void MapeditFrame::LaunchWindow(Map map, const char* filename)
{
  MapeditFrame* frame = new MapeditFrame(map, filename);
  frame->closer = openWindows.insert(end(openWindows), frame);
  frame->Show(true);
}
