#ifndef FRAME_H
#define FRAME_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "map.h"
#include "widget.h"
#include "tile_widget.h"
#include <list>
#include <wx/notebook.h>
#include <memory>
#include <wx/treectrl.h>
#include "undo.h"

class MapPtrCtr : public wxTreeItemData {
  public:
    Map* map;
  
    MapPtrCtr(Map* map) : map(map) {}
};

class MapeditFrame : public wxFrame {
  public:
    MapeditFrame() {}
    MapeditFrame(std::unique_ptr<World> world);
    
    MapeditWidget* GetMapEditor();
    void SetIsAddingEntity(bool isAddingEntity);
    void MapDirtyDidChange(bool dirty);
    void SetIsSettingStart(bool isSettingStart);
    
    static void NewWorld();
    static bool OpenWorld(std::string filename);
    
    std::list<wxWindow*>::iterator closer;

    static void LaunchWindow(std::unique_ptr<World> world);
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
    
    std::unique_ptr<World> world;
    Map* currentMap;
    
    MapeditWidget* mapEditor;
    TileWidget* tileEditor;
    wxToolBar* toolbar;
    wxMenu* menuFile;
    
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
