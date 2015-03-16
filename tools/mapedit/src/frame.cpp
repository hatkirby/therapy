#include "frame.h"
#include "widget.h"
#include "tile_widget.h"
#include <wx/splitter.h>
#include <wx/statline.h>
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
  // Layout 2: Non-splitter between layout 3 and notebook
  // Layout 3: Splitter between map editor and properties editor
  
  wxSplitterWindow* layout3 = new wxSplitterWindow(this, wxID_ANY);
  layout3->SetSashGravity(1.0);
  layout3->SetMinimumPaneSize(20);
  
  notebook = new wxNotebook(this, wxID_ANY);
  
  tileEditor = new TileWidget(notebook, wxID_ANY, 6, 6, wxPoint(0,0), wxSize(TILE_WIDTH*6*6,TILE_HEIGHT*10*6));
  notebook->AddPage(tileEditor, "Tile Chooser", true);
  
  mapEditor = new MapeditWidget(layout3, wxID_ANY, &this->map, tileEditor, wxPoint(0,0), wxSize(GAME_WIDTH*2, GAME_HEIGHT*2));
  mapEditor->frame = this;
  
  // Set up property editor
  wxPanel* propertyEditor = new wxPanel(layout3, wxID_ANY);
  titleBox = new wxTextCtrl(propertyEditor, wxID_ANY, map.getTitle());
  titleBox->Bind(wxEVT_TEXT, &MapeditFrame::OnTitleChange, this);
  
  wxStaticText* titleLabel = new wxStaticText(propertyEditor, wxID_ANY, "Title:");
  
  wxBoxSizer* propertySizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* propertySizer1 = new wxBoxSizer(wxHORIZONTAL);
  propertySizer1->Add(titleLabel, 0, wxALIGN_RIGHT | wxLEFT, 10);
  propertySizer1->Add(titleBox, 1, wxALIGN_LEFT | wxLEFT | wxRIGHT, 10);
  propertySizer->Add(propertySizer1, 1, wxEXPAND | wxTOP, 10);
  propertyEditor->SetSizer(propertySizer);
  propertySizer->SetSizeHints(propertyEditor);
  
  // Set up entity editor
  wxPanel* entityEditor = new wxPanel(notebook, wxID_ANY);
  notebook->AddPage(entityEditor, "Entity Manager", false);
  
  wxStaticText* entityHeader = new wxStaticText(entityEditor, wxID_ANY, "Add Entity");
  wxFont font = entityHeader->GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  entityHeader->SetFont(font);
  
  wxStaticText* entityTypeLabel = new wxStaticText(entityEditor, wxID_ANY, "Entity Type:");
  
  entityTypeBox = new wxChoice(entityEditor, wxID_ANY);
  for (auto entry : MapObject::getAllObjects())
  {
    entityTypeBox->Append(entry.second->getType(), entry.second.get());
  }
  
  addEntityButton = new wxButton(entityEditor, wxID_ANY, "Add Entity");
  addEntityButton->Bind(wxEVT_BUTTON, &MapeditFrame::OnAddEntity, this);
  
  cancelEntityButton = new wxButton(entityEditor, wxID_ANY, "Cancel");
  cancelEntityButton->Disable();
  cancelEntityButton->Bind(wxEVT_BUTTON, &MapeditFrame::OnCancelAddEntity, this);
  
  wxStaticText* entityInfoLabel = new wxStaticText(entityEditor, wxID_ANY, "Click and drag an entity to move it.\nRight click an entity to delete it.");
  
  wxBoxSizer* entitySizer = new wxBoxSizer(wxVERTICAL);
  entitySizer->Add(entityHeader, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  wxBoxSizer* entitySizer1 = new wxBoxSizer(wxHORIZONTAL);
  entitySizer1->Add(entityTypeLabel, 0, wxALIGN_LEFT | wxRIGHT, 5);
  entitySizer1->Add(entityTypeBox, 1, wxALIGN_LEFT, 0);
  entitySizer->Add(entitySizer1, 0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  wxBoxSizer* entitySizer2 = new wxBoxSizer(wxHORIZONTAL);
  entitySizer2->Add(addEntityButton, 1, wxEXPAND | wxRIGHT, 2);
  entitySizer2->Add(cancelEntityButton, 1, wxEXPAND | wxLEFT, 2);
  entitySizer->Add(entitySizer2, 0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  entitySizer->Add(new wxStaticLine(entityEditor), 0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  entitySizer->Add(entityInfoLabel, 0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  entityEditor->SetSizer(entitySizer);
  entitySizer->SetSizeHints(entityEditor);
  
  // Finish setting up the layouts
  layout3->SplitHorizontally(mapEditor, propertyEditor);
  
  notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &MapeditFrame::OnTabChange, this);
  notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGING, &MapeditFrame::OnTabChanging, this);
  
  wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
  sizer2->Add(layout3, 1, wxEXPAND, 0);
  sizer2->Add(notebook, 0, wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxEXPAND, 2);
  this->SetSizer(sizer2);
  sizer2->SetSizeHints(this);
  
  wxBoxSizer* splitterSizer = new wxBoxSizer(wxVERTICAL);
  splitterSizer->Add(layout3, 1, wxEXPAND, 0);
  splitterSizer->Add(mapEditor, 1, wxEXPAND, 0);
  splitterSizer->Add(propertyEditor, 0, wxALIGN_TOP, wxALIGN_LEFT, 0);
  layout3->SetSizer(splitterSizer);
  splitterSizer->SetSizeHints(layout3);
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

void MapeditFrame::ZoomIn(wxCommandEvent&)
{
  mapEditor->ZoomIn();
}

void MapeditFrame::ZoomOut(wxCommandEvent&)
{
  mapEditor->ZoomOut();
}

void MapeditFrame::OnNew(wxCommandEvent&)
{
  NewMap();
}

void MapeditFrame::OnOpen(wxCommandEvent&)
{
  wxFileDialog openFileDialog(this, "Open map", "", "", "XML files (*.xml)|*.xml", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  if (openFileDialog.ShowModal() == wxID_CANCEL)
  {
    return;
  }
  
  OpenMap(openFileDialog.GetPath().c_str());
}

void MapeditFrame::OnSave(wxCommandEvent&)
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

void MapeditFrame::OnClose(wxCommandEvent&)
{
  Close(false);
}

void MapeditFrame::OnQuit(wxCommandEvent&)
{
  for (auto window : openWindows)
  {
    if (window != nullptr)
    {
      window->Close(false);
    }
  }
}

void MapeditFrame::OnTitleChange(wxCommandEvent&)
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

void MapeditFrame::OnTabChange(wxBookCtrlEvent& event)
{
  switch (event.GetSelection())
  {
    case 0:
      mapEditor->SetEditMode(EditTiles);
      break;
      
    case 1:
      mapEditor->SetEditMode(EditEntities);
      break;
  }
  
  event.Skip();
}

void MapeditFrame::OnTabChanging(wxBookCtrlEvent& event)
{
  if (addingEntity)
  {
    event.Veto();
    return;
  }
  
  event.Skip();
}

void MapeditFrame::OnAddEntity(wxCommandEvent&)
{
  addingEntity = true;
  addEntityButton->Disable();
  cancelEntityButton->Enable();
  
  mapEditor->StartAddingEntity((MapObject*) entityTypeBox->GetClientData(entityTypeBox->GetSelection()));
}

void MapeditFrame::OnCancelAddEntity(wxCommandEvent&)
{
  addingEntity = false;
  addEntityButton->Enable();
  cancelEntityButton->Disable();
  
  mapEditor->CancelAddingEntity();
}

void MapeditFrame::StartAddingEntity()
{
  addingEntity = true;
  addEntityButton->Disable();
  cancelEntityButton->Enable();
}

void MapeditFrame::FinishAddingEntity()
{
  addingEntity = false;
  addEntityButton->Enable();
  cancelEntityButton->Disable();
}
