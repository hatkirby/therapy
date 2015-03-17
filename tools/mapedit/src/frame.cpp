#include "frame.h"
#include "widget.h"
#include "tile_widget.h"
#include <wx/statline.h>
#include "panel.h"
#include <list>
#include <exception>
#include <sstream>

static std::list<wxWindow*> openWindows;

enum {
  MENU_VIEW_ZOOM_IN,
  MENU_VIEW_ZOOM_OUT,
  MENU_FILE_NEW,
  MENU_FILE_OPEN,
  MENU_FILE_SAVE,
  MENU_FILE_CLOSE,
  MENU_MAP_ADD_ROOT,
  MENU_MAP_ADD_CHILD,
  MENU_EDIT_UNDO,
  MENU_EDIT_REDO,
  TOOL_FILE_NEW,
  TOOL_FILE_OPEN,
  TOOL_FILE_SAVE,
  TOOL_MAP_ADD_ROOT,
  TOOL_MAP_ADD_CHILD,
  MAP_EDITOR_NOTEBOOK,
  MAP_EDITOR_TREE,
  MAP_TITLE_TEXTBOX,
  ADD_ENTITY_BUTTON,
  CANCEL_ENTITY_BUTTON,
  SET_STARTPOS_BUTTON,
  CANCEL_STARTPOS_BUTTON,
  LAYOUT_ONE_SPLITTER,
  LAYOUT_THREE_SPLITTER
};

wxBEGIN_EVENT_TABLE(MapeditFrame, wxFrame)
  EVT_MENU(wxID_EXIT, MapeditFrame::OnQuit)
  EVT_MENU(MENU_VIEW_ZOOM_IN, MapeditFrame::ZoomIn)
  EVT_MENU(MENU_VIEW_ZOOM_OUT, MapeditFrame::ZoomOut)
  EVT_MENU(MENU_FILE_NEW, MapeditFrame::OnNew)
  EVT_MENU(MENU_FILE_OPEN, MapeditFrame::OnOpen)
  EVT_MENU(MENU_FILE_SAVE, MapeditFrame::OnSave)
  EVT_MENU(MENU_FILE_CLOSE, MapeditFrame::OnClose)
  EVT_MENU(MENU_MAP_ADD_ROOT, MapeditFrame::OnAddRoot)
  EVT_MENU(MENU_MAP_ADD_CHILD, MapeditFrame::OnAddChild)
  EVT_MENU(MENU_EDIT_UNDO, MapeditFrame::OnUndo)
  EVT_MENU(MENU_EDIT_REDO, MapeditFrame::OnRedo)
  EVT_TOOL(TOOL_FILE_NEW, MapeditFrame::OnNew)
  EVT_TOOL(TOOL_FILE_OPEN, MapeditFrame::OnOpen)
  EVT_TOOL(TOOL_FILE_SAVE, MapeditFrame::OnSave)
  EVT_TOOL(TOOL_MAP_ADD_ROOT, MapeditFrame::OnAddRoot)
  EVT_TOOL(TOOL_MAP_ADD_CHILD, MapeditFrame::OnAddChild)
  EVT_CLOSE(MapeditFrame::OnExit)
  EVT_NOTEBOOK_PAGE_CHANGED(MAP_EDITOR_NOTEBOOK, MapeditFrame::OnTabChange)
  EVT_NOTEBOOK_PAGE_CHANGING(MAP_EDITOR_NOTEBOOK, MapeditFrame::OnTabChanging)
  EVT_TREE_SEL_CHANGED(MAP_EDITOR_TREE, MapeditFrame::OnDidSelectMap)
  EVT_TREE_BEGIN_DRAG(MAP_EDITOR_TREE, MapeditFrame::OnWillDragMap)
  EVT_TREE_END_DRAG(MAP_EDITOR_TREE, MapeditFrame::OnDidDragMap)
  EVT_TREE_ITEM_RIGHT_CLICK(MAP_EDITOR_TREE, MapeditFrame::OnRightClickTree)
  EVT_TEXT(MAP_TITLE_TEXTBOX, MapeditFrame::OnTitleChange)
  EVT_BUTTON(ADD_ENTITY_BUTTON, MapeditFrame::OnAddEntity)
  EVT_BUTTON(CANCEL_ENTITY_BUTTON, MapeditFrame::OnCancelAddEntity)
  EVT_BUTTON(SET_STARTPOS_BUTTON, MapeditFrame::OnSetStartpos)
  EVT_BUTTON(CANCEL_STARTPOS_BUTTON, MapeditFrame::OnCancelSetStartpos)
  EVT_SPLITTER_SASH_POS_CHANGING(LAYOUT_ONE_SPLITTER, MapeditFrame::OnOneMovingSash)
  EVT_SPLITTER_SASH_POS_CHANGING(LAYOUT_THREE_SPLITTER, MapeditFrame::OnThreeMovingSash)
wxEND_EVENT_TABLE()

MapeditFrame::MapeditFrame(std::unique_ptr<World> world) : wxFrame(NULL, wxID_ANY, "Map Editor")
{
  int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
  int screenHeight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
  bool setSize = false;
  wxSize toSetSize = GetSize();
  if (screenWidth > 1280)
  {
    setSize = true;
    toSetSize.SetWidth(1280);
  }
  
  if (screenHeight > 800)
  {
    setSize = true;
    toSetSize.SetHeight(800);
  }
  
  if (setSize)
  {
    SetSize(toSetSize);
    
    int numClosers = openWindows.size() - 1;
    SetPosition({GetPosition().x + numClosers*20, GetPosition().y + numClosers*20});
  } else {
    Maximize();
  }
  
  this->world = std::move(world);
  this->world->setParent(this);
  currentMap = this->world->getLastMap();
  
  menuFile = new wxMenu;
  menuFile->Append(MENU_FILE_NEW, "New\tCtrl-N");
  menuFile->Append(MENU_FILE_OPEN, "Open\tCtrl-O");
  menuFile->Append(MENU_FILE_SAVE, "Save\tCtrl-S");
  menuFile->Append(MENU_FILE_CLOSE, "Close\tCtrl-W");
  menuFile->AppendSeparator();
  menuFile->Append(MENU_MAP_ADD_ROOT, "New Map\tCtrl-Alt-N");
  menuFile->Append(MENU_MAP_ADD_CHILD, "New Child Map\tCtrl-Alt-Shift-N");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);
  
  menuEdit = new wxMenu;
  menuEdit->Append(MENU_EDIT_UNDO, "Undo\tCtrl-Z");
  menuEdit->Append(MENU_EDIT_REDO, "Redo\tCtrl-Shift-Z");
  UpdateUndoLabels();
  
  wxMenu* menuView = new wxMenu;
  menuView->Append(MENU_VIEW_ZOOM_IN, "Zoom In\tCtrl-+");
  menuView->Append(MENU_VIEW_ZOOM_OUT, "Zoom Out\tCtrl--");

  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuEdit, "&Edit");
  menuBar->Append(menuView, "&View");
  
  SetMenuBar(menuBar);
  
  mapTreePopup = new wxMenu;
  mapTreePopup->Append(MENU_MAP_ADD_ROOT, "Add Sibling Map");
  mapTreePopup->Append(MENU_MAP_ADD_CHILD, "Add Child Map");
  
  // Layout 1: Splitter between map tree and layout 2
  // Layout 2: Non-splitter between layout 3 and notebook
  // Layout 3: Splitter between map editor and properties editor
  
  layout1 = new wxSplitterWindow(this, LAYOUT_ONE_SPLITTER);
  mapTree = new wxTreeCtrl(layout1, MAP_EDITOR_TREE, wxDefaultPosition, wxSize(200, -1), wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS);
  wxTreeItemId mapTreeRoot = mapTree->AddRoot("root");
  populateMapTree(mapTreeRoot, this->world->getRootMaps());
  
  wxPanel* layout2 = new wxPanel(layout1, wxID_ANY);
  
  layout3 = new wxSplitterWindow(layout2, LAYOUT_THREE_SPLITTER);
  layout3->SetSashGravity(1.0);
  
  notebook = new wxNotebook(layout2, MAP_EDITOR_NOTEBOOK);
  
  tileEditor = new TileWidget(notebook, wxID_ANY, 6, 6, wxPoint(0,0), wxSize(TILE_WIDTH*6*6,TILE_HEIGHT*10*6));
  notebook->AddPage(tileEditor, "Tile Chooser", false);
  
  mapEditor = new MapeditWidget(layout3, wxID_ANY, currentMap, tileEditor, wxPoint(0,0), wxSize(GAME_WIDTH*2, GAME_HEIGHT*2));
  mapEditor->frame = this;
  
  // Set up property editor
  propertyEditor = new wxPanel(layout3, wxID_ANY);//, wxDefaultPosition, wxSize(-1, 100));
  titleBox = new UndoableTextBox(propertyEditor, MAP_TITLE_TEXTBOX, currentMap->getTitle(), "Edit Map Title", this);
  titleBox->SetMaxLength(40);
  
  wxStaticText* titleLabel = new wxStaticText(propertyEditor, wxID_ANY, "Title:");
  
  startposLabel = new wxStaticText(propertyEditor, wxID_ANY, "Starting Position:");
  
  setStartposButton = new wxButton(propertyEditor, SET_STARTPOS_BUTTON, "Set Starting Position");
  cancelStartposButton = new wxButton(propertyEditor, CANCEL_STARTPOS_BUTTON, "Cancel");
  cancelStartposButton->Disable();
  
  wxBoxSizer* propertySizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* propertySizer1 = new wxBoxSizer(wxHORIZONTAL);
  propertySizer1->Add(titleLabel, 0, wxALIGN_RIGHT | wxLEFT, 10);
  propertySizer1->Add(titleBox, 1, wxALIGN_LEFT | wxLEFT | wxRIGHT, 10);
  propertySizer->Add(propertySizer1, 0, wxEXPAND | wxTOP, 10);
  wxBoxSizer* propertySizer2 = new wxBoxSizer(wxHORIZONTAL);
  propertySizer2->Add(startposLabel, 0, wxALIGN_RIGHT | wxLEFT, 10);
  propertySizer2->Add(setStartposButton, 0, wxALIGN_LEFT | wxLEFT, 10);
  propertySizer2->Add(cancelStartposButton, 0, wxALIGN_LEFT | wxLEFT, 10);
  propertySizer->Add(propertySizer2, 0, wxEXPAND | wxTOP | wxBOTTOM, 10);
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
  
  addEntityButton = new wxButton(entityEditor, ADD_ENTITY_BUTTON, "Add Entity");
  cancelEntityButton = new wxButton(entityEditor, CANCEL_ENTITY_BUTTON, "Cancel");
  cancelEntityButton->Disable();
  
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
  wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
  sizer1->Add(layout1, 1, wxEXPAND, 0);
  sizer1->Add(mapTree, 0, wxALIGN_TOP | wxALIGN_LEFT, 0);
  sizer1->Add(layout2, 2, wxEXPAND, 0);
  layout1->SetSizer(sizer1);
  sizer1->SetSizeHints(layout1);
  
  wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
  sizer2->Add(layout3, 1, wxEXPAND, 0);
  sizer2->Add(notebook, 0, wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxEXPAND, 2);
  layout2->SetSizer(sizer2);
  sizer2->SetSizeHints(layout2);
  
  wxBoxSizer* splitterSizer = new wxBoxSizer(wxVERTICAL);
  splitterSizer->Add(layout3, 0, wxEXPAND, 0);
  splitterSizer->Add(mapEditor, 1, wxEXPAND, 0);
  splitterSizer->Add(propertyEditor, 0, wxALIGN_BOTTOM | wxALIGN_LEFT, 0);
  layout3->SetSizer(splitterSizer);
  splitterSizer->SetSizeHints(layout3);
  
  layout3->SplitHorizontally(mapEditor, propertyEditor, -propertyEditor->GetSize().GetHeight());
  layout1->SplitVertically(mapTree, layout2, mapTree->GetSize().GetWidth());
  
  // Toolbar time!
  toolbar = CreateToolBar();
  toolbar->AddTool(TOOL_FILE_NEW, "New", wxBitmap(wxImage("res/page.png")));
  toolbar->AddTool(TOOL_FILE_OPEN, "Open", wxBitmap(wxImage("res/folder_page.png")));
  toolbar->AddTool(TOOL_FILE_SAVE, "Save", wxBitmap(wxImage("res/disk.png")));
  toolbar->AddSeparator();
  toolbar->AddTool(TOOL_MAP_ADD_ROOT, "Add Map", wxBitmap(wxImage("res/page_add.png")));
  toolbar->AddTool(TOOL_MAP_ADD_CHILD, "Add Child Map", wxBitmap(wxImage("res/page_white_add.png")));
  toolbar->EnableTool(TOOL_FILE_SAVE, this->world->getDirty());
  toolbar->Realize();
  
  dontSelectMap = true;
  mapTree->SelectItem(currentMap->getTreeItemId());
  dontSelectMap = false;
  
  SetStartposLabel();
}

void MapeditFrame::OnExit(wxCloseEvent& event)
{
  if (event.CanVeto() && world->getDirty())
  {
    switch (wxMessageBox("One or more maps have unsaved changes. Save before closing?", "Please confirm", wxICON_QUESTION|wxYES_NO|wxCANCEL, this))
    {
      case wxYES:
        if (world->getFilename() == "")
        {
          wxFileDialog saveFileDialog(this, "Save world", "", "", "XML files (*.xml)|*.xml", wxFD_SAVE);
          if (saveFileDialog.ShowModal() == wxID_CANCEL)
          {
            return;
          }
    
          world->save(saveFileDialog.GetPath().ToStdString(), mapTree);
        } else {
          world->save(world->getFilename(), mapTree);
        }
  
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
  NewWorld();
}

void MapeditFrame::OnOpen(wxCommandEvent&)
{
  wxFileDialog openFileDialog(this, "Open world", "", "", "XML files (*.xml)|*.xml", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  if (openFileDialog.ShowModal() == wxID_CANCEL)
  {
    return;
  }
  
  if (OpenWorld(openFileDialog.GetPath().ToStdString()))
  {
    if (world->getEmpty())
    {
      Close(true);
    }
  }
}

void MapeditFrame::OnSave(wxCommandEvent&)
{
  if (world->getFilename() == "")
  {
    wxFileDialog saveFileDialog(this, "Save world", "", "", "XML files (*.xml)|*.xml", wxFD_SAVE);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
    {
      return;
    }
    
    world->save(saveFileDialog.GetPath().ToStdString(), mapTree);
  } else {
    world->save(world->getFilename(), mapTree);
  }
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

void MapeditFrame::OnTitleChange(wxCommandEvent& event)
{
  currentMap->setTitle(titleBox->GetValue().ToStdString());
  mapTree->SetItemText(currentMap->getTreeItemId(), titleBox->GetValue());
  
  event.Skip();
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

void MapeditFrame::OnAddRoot(wxCommandEvent&)
{
  auto map = world->newMap();
  
  commitAction(std::make_shared<Undoable>("New Map", [=] () {
    map->setHidden(false);
    
    wxTreeItemId sel = mapTree->GetSelection();
    wxTreeItemId par = mapTree->GetItemParent(sel);
    wxTreeItemId node = mapTree->AppendItem(par, map->getTitle());
    map->setTreeItemId(node);
    mapTree->SetItemData(node, new MapPtrCtr(map.get()));
    
    dontSelectMap = true;
    mapTree->SelectItem(node);
    dontSelectMap = false;
  }, [=] () {
    map->setHidden(true);
    
    wxTreeItemId sel = mapTree->GetPrevSibling(map->getTreeItemId());
    mapTree->Delete(map->getTreeItemId());
    
    dontSelectMap = true;
    mapTree->SelectItem(sel);
    dontSelectMap = false;
  }));
  
}

void MapeditFrame::OnAddChild(wxCommandEvent&)
{
  auto map = world->newMap();
  
  commitAction(std::make_shared<Undoable>("New Map", [=] () {
    map->setHidden(false);
    
    wxTreeItemId sel = mapTree->GetSelection();
    wxTreeItemId node = mapTree->AppendItem(sel, map->getTitle());
    map->setTreeItemId(node);
    mapTree->SetItemData(node, new MapPtrCtr(map.get()));
    
    mapTree->Expand(sel);
    
    dontSelectMap = true;
    mapTree->SelectItem(node);
    dontSelectMap = false;
  }, [=] () {
    map->setHidden(true);
    
    wxTreeItemId sel = mapTree->GetItemParent(map->getTreeItemId());
    mapTree->Delete(map->getTreeItemId());
    
    dontSelectMap = true;
    mapTree->SelectItem(sel);
    dontSelectMap = false;
  }));
}

void MapeditFrame::OnDidSelectMap(wxTreeEvent& event)
{
  MapPtrCtr* data = (MapPtrCtr*) mapTree->GetItemData(event.GetItem());
  SelectMap(data->map);
  
  if (!dontSelectMap)
  {
    commitAfter(std::make_shared<Undoable>("Selecting " + data->map->getTitle(), [=] () {
      wxTreeItemId toSelect = event.GetItem();
      dontSelectMap = true;
      mapTree->SelectItem(toSelect);
      dontSelectMap = false;
      SelectMap(data->map);
    }, [=] () {
      wxTreeItemId toSelect = event.GetOldItem();
      MapPtrCtr* oldData = (MapPtrCtr*) mapTree->GetItemData(toSelect);
      dontSelectMap = true;
      mapTree->SelectItem(toSelect);
      dontSelectMap = false;
      SelectMap(oldData->map);
    }));
  }
}

void MapeditFrame::OnWillDragMap(wxTreeEvent& event)
{
  event.Allow();
  dragMap = event.GetItem();
}

void MapeditFrame::OnDidDragMap(wxTreeEvent& event)
{
  if (!dragMap.IsOk())
  {
    return;
  }
  
  wxTreeItemId newParent = event.GetItem();
  if (!newParent.IsOk())
  {
    newParent = mapTree->GetRootItem();
  }
  
  wxTreeItemId curParent = mapTree->GetItemParent(event.GetItem());
  wxTreeItemId dragMapCopy = dragMap;
  dragMap.Unset();
  
  Map* theMap = ((MapPtrCtr*) mapTree->GetItemData(dragMap))->map;
  commitAction(std::make_shared<Undoable>("Arranging " + theMap->getTitle(), [=] () {
    wxTreeItemId newChild = MoveTreeNode(dragMapCopy, newParent);
  
    dontSelectMap = true;
    mapTree->SelectItem(newChild);
    dontSelectMap = false;
  }, [=] () {
    wxTreeItemId newChild = MoveTreeNode(dragMapCopy, curParent);
  
    dontSelectMap = true;
    mapTree->SelectItem(newChild);
    dontSelectMap = false;
  }));
}

void MapeditFrame::OnRightClickTree(wxTreeEvent& event)
{
  if (event.GetItem().IsOk())
  {
    PopupMenu(mapTreePopup);
  }
}

void MapeditFrame::OnSetStartpos(wxCommandEvent&)
{
  SetIsSettingStart(true);
  mapEditor->SetIsSettingStart(true);
}

void MapeditFrame::OnCancelSetStartpos(wxCommandEvent&)
{
  SetIsSettingStart(false);
  mapEditor->SetIsSettingStart(false);
}

void MapeditFrame::OnUndo(wxCommandEvent&)
{
  (*currentAction)->endChanges();
  (*currentAction)->undo();
  currentAction++;
  
  UpdateUndoLabels();
}

void MapeditFrame::OnRedo(wxCommandEvent&)
{
  if (currentAction != end(history))
  {
    (*currentAction)->endChanges();
  }
  
  currentAction--;
  (*currentAction)->apply();
  
  UpdateUndoLabels();
}

void MapeditFrame::OnOneMovingSash(wxSplitterEvent& event)
{
  layout1->SetSashPosition(event.GetSashPosition(), true);
}

void MapeditFrame::OnThreeMovingSash(wxSplitterEvent& event)
{
  layout3->SetSashPosition(event.GetSashPosition(), true);
}

void MapeditFrame::NewWorld()
{
  LaunchWindow(std::unique_ptr<World>(new World()));
}

bool MapeditFrame::OpenWorld(std::string filename)
{
  try
  {
    auto world = std::unique_ptr<World>(new World(filename));
    
    LaunchWindow(std::move(world));
    
    return true;
  } catch (std::exception& ex)
  {
    wxMessageBox(ex.what(), "Error loading world", wxOK | wxCENTRE | wxICON_ERROR);
  }
  
  return false;
}

void MapeditFrame::LaunchWindow(std::unique_ptr<World> world)
{
  MapeditFrame* frame = new MapeditFrame(std::move(world));
  frame->closer = openWindows.insert(end(openWindows), frame);
  frame->Show(true);
}

void MapeditFrame::SetIsAddingEntity(bool isAddingEntity)
{
  if (isAddingEntity)
  {
    addingEntity = true;
    addEntityButton->Disable();
    cancelEntityButton->Enable();
  } else {
    addingEntity = false;
    addEntityButton->Enable();
    cancelEntityButton->Disable();
  }
}

void MapeditFrame::MapDirtyDidChange(bool dirty)
{
  toolbar->EnableTool(TOOL_FILE_SAVE, dirty);
  menuFile->Enable(MENU_FILE_SAVE, dirty);
  
  if (dirty)
  {
    mapTree->SetItemBold(currentMap->getTreeItemId(), true);
  } else {
    for (auto map : world->getMaps())
    {
      mapTree->SetItemBold(map.second->getTreeItemId(), false);
    }
  }
}

void MapeditFrame::populateMapTree(wxTreeItemId node, std::list<std::shared_ptr<Map>> maps)
{
  for (auto map : maps)
  {
    wxTreeItemId childNode = mapTree->AppendItem(node, map->getTitle());
    mapTree->SetItemData(childNode, new MapPtrCtr(map.get()));
    map->setTreeItemId(childNode);
    
    populateMapTree(childNode, map->getChildren());
    
    if (map->getExpanded())
    {
      mapTree->Expand(childNode);
    }
  }
}

void MapeditFrame::SelectMap(Map* map)
{
  currentMap = map;
  mapEditor->SetMap(map);
  
  SetIsAddingEntity(false);
  SetIsSettingStart(false);
  
  titleBox->ChangeValue(map->getTitle());
  world->setLastMap(map);
}

wxTreeItemId MapeditFrame::MoveTreeNode(wxTreeItemId toCopy, wxTreeItemId newParent)
{
  MapPtrCtr* ctl1 = (MapPtrCtr*) mapTree->GetItemData(toCopy);
  MapPtrCtr* ctl2 = new MapPtrCtr(ctl1->map);
  
  wxTreeItemId copied = mapTree->AppendItem(newParent, mapTree->GetItemText(toCopy), -1, -1, ctl2);
  if (mapTree->IsBold(toCopy))
  {
    mapTree->SetItemBold(toCopy, true);
  }
  
  if (mapTree->ItemHasChildren(toCopy))
  {
    wxTreeItemIdValue cookie;
    for (wxTreeItemId it = mapTree->GetFirstChild(toCopy, cookie); it.IsOk(); it = mapTree->GetNextChild(toCopy, cookie))
    {
      MoveTreeNode(it, copied);
    }
  }
  
  if (mapTree->IsExpanded(toCopy))
  {
    mapTree->Expand(copied);
  }
  
  mapTree->Delete(toCopy);
  
  return copied;
}

void MapeditFrame::SetIsSettingStart(bool isSettingStart)
{
  if (isSettingStart)
  {
    setStartposButton->Disable();
    cancelStartposButton->Enable();
  } else {
    SetStartposLabel();
    
    setStartposButton->Enable();
    cancelStartposButton->Disable();
  }
}

void MapeditFrame::SetStartposLabel()
{
  std::ostringstream mappos_out;
  mappos_out << "Starting Position: ";
  mappos_out << world->getStartingMap()->getTitle();
  mappos_out << " (";
  mappos_out << (int) world->getStartingPosition().first;
  mappos_out << ",";
  mappos_out << (int) world->getStartingPosition().second;
  mappos_out << ")";
  
  startposLabel->SetLabel(mappos_out.str());
  propertyEditor->GetSizer()->SetSizeHints(propertyEditor);
}

void MapeditFrame::UpdateUndoLabels()
{
  if (currentAction != end(history))
  {
    menuEdit->SetLabel(MENU_EDIT_UNDO, "Undo " + (*currentAction)->getTitle() + "\tCtrl-Z");
    menuEdit->Enable(MENU_EDIT_UNDO, true);
  } else {
    menuEdit->SetLabel(MENU_EDIT_UNDO, "Undo\tCtrl-Z");
    menuEdit->Enable(MENU_EDIT_UNDO, false);
  }
  
  if (currentAction != begin(history))
  {
    menuEdit->SetLabel(MENU_EDIT_REDO, "Redo " + (*std::prev(currentAction))->getTitle() + "\tCtrl-Shift-Z");
    menuEdit->Enable(MENU_EDIT_REDO, true);
  } else {
    menuEdit->SetLabel(MENU_EDIT_REDO, "Redo\tCtrl-Shift-Z");
    menuEdit->Enable(MENU_EDIT_REDO, false);
  }
}

void MapeditFrame::commitAction(std::shared_ptr<Undoable> action)
{
  action->apply();
  
  commitAfter(action);
}

void MapeditFrame::commitAfter(std::shared_ptr<Undoable> action)
{
  if (currentAction != end(history))
  {
    (*currentAction)->endChanges();
  }

  history.erase(begin(history), currentAction);
  currentAction = history.insert(begin(history), action);
  
  UpdateUndoLabels();
  
  if (history.size() > 100)
  {
    history.pop_back();
  }
}
