#ifndef FRAME_H
#define FRAME_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <list>
#include <wx/notebook.h>
#include <memory>
#include <wx/treectrl.h>
#include <wx/splitter.h>

class Map;
class MapeditWidget;
class TileWidget;
class Undoable;
class UndoableTextBox;

#include "world.h"

class MapeditFrame : public wxFrame {
  public:
    MapeditFrame() {}
    MapeditFrame(World* world);
    ~MapeditFrame() { delete world; }
    
    MapeditWidget* GetMapEditor();
    void SetIsAddingEntity(bool isAddingEntity);
    void MapDirtyDidChange(bool dirty);
    void SetIsSettingStart(bool isSettingStart);
    
    static void NewWorld();
    static bool OpenWorld(std::string filename);
    
    std::list<wxWindow*>::iterator closer;

    static void LaunchWindow(World* world);
    void populateMapTree(wxTreeItemId node, std::list<std::shared_ptr<Map>> maps);
    void SelectMap(Map* map);
    wxTreeItemId MoveTreeNode(wxTreeItemId toCopy, wxTreeItemId newParent);
    void SetStartposLabel();
      
    void ZoomIn(wxCommandEvent& event);
    void ZoomOut(wxCommandEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnExit(wxCloseEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnTitleChange(wxCommandEvent& event);
    void OnTabChange(wxBookCtrlEvent& event);
    void OnTabChanging(wxBookCtrlEvent& event);
    void OnAddEntity(wxCommandEvent& event);
    void OnCancelAddEntity(wxCommandEvent& event);
    void OnAddRoot(wxCommandEvent& event);
    void OnAddChild(wxCommandEvent& event);
    void OnDidSelectMap(wxTreeEvent& event);
    void OnWillDragMap(wxTreeEvent& event);
    void OnDidDragMap(wxTreeEvent& event);
    void OnRightClickTree(wxTreeEvent& event);
    void OnSetStartpos(wxCommandEvent& event);
    void OnCancelSetStartpos(wxCommandEvent& event);
    void OnOneMovingSash(wxSplitterEvent& event);
    void OnThreeMovingSash(wxSplitterEvent& event);
    void OnSetLeftmapType(wxCommandEvent& event);
    void OnSetLeftmapMap(wxCommandEvent& event);
    void OnSetRightmapType(wxCommandEvent& event);
    void OnSetRightmapMap(wxCommandEvent& event);
    void OnSetUpmapType(wxCommandEvent& event);
    void OnSetUpmapMap(wxCommandEvent& event);
    void OnSetDownmapType(wxCommandEvent& event);
    void OnSetDownmapMap(wxCommandEvent& event);
    void OnSelectEntity(wxCommandEvent& event);
    
    World* world;
    Map* currentMap;
    
    MapeditWidget* mapEditor;
    TileWidget* tileEditor;
    wxToolBar* toolbar;
    wxMenu* menuFile;
    wxSplitterWindow* layout1;
    wxSplitterWindow* layout3;
    wxPanel* propertyEditor;
    
    // Notebook
    wxNotebook* notebook;
    wxChoice* entityTypeBox;
    wxButton* addEntityButton;
    wxButton* cancelEntityButton;

    // Map tree
    wxTreeCtrl* mapTree;
    wxTreeItemId dragMap;
    wxMenu* mapTreePopup;
    bool dontSelectMap = false;
    
    // Property editor
    UndoableTextBox* titleBox;
    wxString prevTitle;
    wxStaticText* startposLabel;
    wxButton* setStartposButton;
    wxButton* cancelStartposButton;
    
    // Undo stuff
    wxMenu* menuEdit;
    std::list<std::shared_ptr<Undoable>> history;
    std::list<std::shared_ptr<Undoable>>::iterator currentAction {begin(history)};
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void UpdateUndoLabels();
    void commitAction(std::shared_ptr<Undoable> action);
    void commitAfter(std::shared_ptr<Undoable> action);
    
    bool addingEntity = false;
    
  private:
    wxDECLARE_EVENT_TABLE();
};

#endif
