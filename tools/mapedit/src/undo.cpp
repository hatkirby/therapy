#include "undo.h"
#include "frame.h"

Undoable::Undoable(std::string title, std::function<void ()> forward, std::function<void ()> backward)
{
  this->title = title;
  this->forward = forward;
  this->backward = backward;
}

std::string Undoable::getTitle() const
{
  return title;
}

void Undoable::apply()
{
  forward();
}

void Undoable::undo()
{
  backward();
}

wxBEGIN_EVENT_TABLE(UndoableTextBox, wxTextCtrl)
  EVT_SET_FOCUS(UndoableTextBox::OnFocus)
  EVT_KILL_FOCUS(UndoableTextBox::OnKillFocus)
wxEND_EVENT_TABLE()

UndoableTextBox::UndoableTextBox(wxWindow* parent, wxWindowID id, wxString startText, std::string undoType, MapeditFrame* realParent, wxPoint pos, wxSize size, long style) : wxTextCtrl(parent, id, startText, pos, size, style)
{
  this->undoText = undoType;
  this->realParent = realParent;
  
  Init();
}

void UndoableTextBox::Init()
{
  Bind(wxEVT_TEXT, &UndoableTextBox::OnTextChange, this);
  
  curText = GetValue();
}

void UndoableTextBox::OnFocus(wxFocusEvent& event)
{
  curText = GetValue();
  
  event.Skip();
}

void UndoableTextBox::OnKillFocus(wxFocusEvent& event)
{
  undo.reset();
  
  event.Skip();
}

void UndoableTextBox::OnTextChange(wxCommandEvent& event)
{
  if (!undo)
  {
    auto tempUndo = std::make_shared<Undo>(undoText, curText, *this);
    
    realParent->commitAfter(tempUndo);
    
    undo = tempUndo;
  }
  
  undo->newText = GetValue();
  curText = GetValue();
  
  event.Skip();
}

UndoableTextBox::Undo::Undo(std::string title, wxString origText, UndoableTextBox& parent) : origText(origText), parent(parent)
{
  this->title = title;
}

void UndoableTextBox::Undo::apply()
{
  parent.ChangeValue(newText);
}

void UndoableTextBox::Undo::undo()
{
  parent.ChangeValue(origText);
}

void UndoableTextBox::Undo::endChanges()
{
  parent.undo.reset();
}
