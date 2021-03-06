#ifndef UNDO_H
#define UNDO_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <functional>
#include <string>
#include <list>

class MapeditFrame;

class Undoable {
  public:
    Undoable(std::string title, std::function<void ()> forward, std::function<void ()> backward);
    virtual std::string getTitle() const;
    virtual void apply();
    virtual void undo();
    virtual void endChanges() {}
    
  protected:
    Undoable() {}
    
    std::string title;
    std::function<void ()> forward;
    std::function<void ()> backward;
};

class UndoableTextBox : public wxTextCtrl {
  public:
    UndoableTextBox();
    UndoableTextBox(wxWindow* parent, wxWindowID id, wxString startText, std::string undoType, MapeditFrame* realParent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
    
  private:
    class Undo : public Undoable {
      public:
        Undo(std::string title, wxString origText, UndoableTextBox& parent);
        void apply();
        void undo();
        void endChanges();
      
        wxString origText;
        wxString newText;
        UndoableTextBox& parent;
    };
    
    void Init();
    void OnFocus(wxFocusEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnTextChange(wxCommandEvent& event);
    
    MapeditFrame* realParent;
    std::string undoText;
    wxString curText;
    std::shared_ptr<Undo> undo;
    
    wxDECLARE_EVENT_TABLE();
};

class UndoableChoice : public wxChoice {
  public:
    UndoableChoice();
    UndoableChoice(wxWindow* parent, wxWindowID id, MapeditFrame* realParent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxChoiceNameStr);
    
  protected:
    void Init();
    void OnChoose(wxCommandEvent& event);
    
  private:
    MapeditFrame* realParent;
};

class UndoableSlider : public wxSlider {
  public:
    UndoableSlider();
    UndoableSlider(wxWindow* parent, wxWindowID id, MapeditFrame* realParent, int value, int minvalue, int maxvalue, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxSliderNameStr);
    
  protected:
    void Init();
    void OnSlide(wxScrollEvent& event);
    
  private:
    MapeditFrame* realParent;
    
    wxDECLARE_EVENT_TABLE();
};

#endif
