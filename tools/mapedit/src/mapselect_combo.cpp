#include "mapselect_combo.h"
#include "map.h"

wxBEGIN_EVENT_TABLE(MapSelectComboPopup, wxTreeCtrl)
  EVT_LEFT_UP(MapSelectComboPopup::OnMouseClick)
wxEND_EVENT_TABLE()

MapSelectComboPopup::MapSelectComboPopup(wxTreeCtrl* tree, int initial)
{
  mapTree = tree;
  lastSelected = initial;

  Init();
}

void MapSelectComboPopup::Init()
{

}

bool MapSelectComboPopup::Create(wxWindow* parent)
{
  return wxTreeCtrl::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HIDE_ROOT);
}

wxWindow* MapSelectComboPopup::GetControl()
{
  return this;
}

wxString MapSelectComboPopup::GetStringValue() const
{
  return ((MapPtrCtr*) GetItemData(GetFocusedItem()))->map->getTitle();
}

void MapSelectComboPopup::OnPopup()
{
  DeleteAllItems();
  
  wxTreeItemId newRoot = AddRoot("root");
  wxTreeItemId oldRoot = mapTree->GetRootItem();
  wxTreeItemIdValue cookie;
  for (wxTreeItemId it = mapTree->GetFirstChild(oldRoot, cookie); it.IsOk(); it = mapTree->GetNextChild(newRoot, cookie))
  {
    CopyNodes(it, newRoot);
  }
}

void MapSelectComboPopup::CopyNodes(wxTreeItemId from, wxTreeItemId to)
{
  MapPtrCtr* ctl1 = (MapPtrCtr*) mapTree->GetItemData(from);
  MapPtrCtr* ctl2 = nullptr;
  if (ctl1 != nullptr)
  {
    ctl2 = new MapPtrCtr(ctl1->map);
  }
  
  wxTreeItemId copied = mapTree->AppendItem(to, mapTree->GetItemText(from), -1, -1, ctl2);
  if (mapTree->ItemHasChildren(from))
  {
    wxTreeItemIdValue cookie;
    for (wxTreeItemId it = mapTree->GetFirstChild(from, cookie); it.IsOk(); it = mapTree->GetNextChild(from, cookie))
    {
      CopyNodes(it, copied);
    }
  }
  
  if (ctl1->map->getID() == lastSelected)
  {
    SelectItem(copied);
  }
  
  Expand(copied);
}

void MapSelectComboPopup::OnMouseClick(wxMouseEvent&)
{
  lastSelected = ((MapPtrCtr*) GetItemData(GetFocusedItem()))->map->getID();
    
  Dismiss();
}

int MapSelectComboPopup::GetSelectedMapID() const
{
  return lastSelected;
}

void MapSelectComboPopup::SetSelectedMapID(int id)
{
  lastSelected = id;
}
