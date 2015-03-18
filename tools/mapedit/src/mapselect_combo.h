#ifndef MAPSELECT_COMBO_H
#define MAPSELECT_COMBO_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/combo.h>
#include <wx/treectrl.h>

class MapeditFrame;

class MapSelectComboPopup : public wxTreeCtrl, public wxComboPopup {
  public:
    MapSelectComboPopup(wxTreeCtrl* mapTree, int initial);
    void Init();
    bool Create(wxWindow* parent);
    wxWindow* GetControl();
    wxString GetStringValue() const;
    void OnPopup();
    void OnMouseClick(wxMouseEvent& event);
    int GetSelectedMapID() const;
    void SetSelectedMapID(int id);
    
  private:
    void CopyNodes(wxTreeItemId from, wxTreeItemId to);
    
    wxTreeCtrl* mapTree;
    int lastSelected = 0;
    
    wxDECLARE_EVENT_TABLE();
};

#endif
