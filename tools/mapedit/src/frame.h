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

  private:
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
    void OnWillSelectMap(wxTreeEvent& event);
    void OnWillDragMap(wxTreeEvent& event);
    void OnDidDragMap(wxTreeEvent& event);
    void OnRightClickTree(wxTreeEvent& event);
    void OnSetStartpos(wxCommandEvent& event);
    void OnCancelSetStartpos(wxCommandEvent& event);
    
    std::unique_ptr<World> world;
    Map* currentMap;
    MapeditWidget* mapEditor;
    TileWidget* tileEditor;
    wxTextCtrl* titleBox;
    std::string filename;
    wxNotebook* notebook;
    wxChoice* entityTypeBox;
    wxButton* addEntityButton;
    wxButton* cancelEntityButton;
    wxToolBar* toolbar;
    wxMenu* menuFile;
    wxTreeCtrl* mapTree;
    wxTreeItemId dragMap;
    wxMenu* mapTreePopup;
    wxStaticText* startposLabel;
    wxButton* setStartposButton;
    wxButton* cancelStartposButton;
    
    bool addingEntity = false;
    
    wxDECLARE_EVENT_TABLE();
};

#endif
