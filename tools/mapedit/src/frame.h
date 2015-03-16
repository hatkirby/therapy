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

class MapeditFrame : public wxFrame {
  public:
    MapeditFrame() {}
    MapeditFrame(Map map, std::string filename);
    
    MapeditWidget* GetMapEditor();
    void StartAddingEntity();
    void FinishAddingEntity();
    void MapDirtyDidChange(bool dirty);
    
    static void NewMap();
    static void OpenMap(const char* filename);
    
    std::list<wxWindow*>::iterator closer;

  private:
    static void LaunchWindow(Map map, const char* filename);
      
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
    
    Map map;
    MapeditWidget* mapEditor;
    TileWidget* tileEditor;
    wxTextCtrl* titleBox;
    std::string filename;
    wxNotebook* notebook;
    wxChoice* entityTypeBox;
    wxButton* addEntityButton;
    wxButton* cancelEntityButton;
    wxToolBar* toolbar;
    
    bool addingEntity = false;
    
    wxDECLARE_EVENT_TABLE();
};

#endif
