/******************************************************************************
 * @brief   TWins - IWindowState definition
 * @author  Mariusz Midor
 *          https://bitbucket.org/mmidor/twins
 *****************************************************************************/

#pragma once

// -----------------------------------------------------------------------------

/** @brief Forward declaration */
struct Widget;

template <class T>
class Vector;

/** @brief Window state and event handler */
class IWindowState
{
public:
    virtual ~IWindowState() = default;
    // events
    virtual void onButtonDown(const twins::Widget* pWgt) {}
    virtual void onButtonUp(const twins::Widget* pWgt) {}
    virtual void onEditChange(const twins::Widget* pWgt, twins::String &&str) {}
    virtual bool onEditInputEvt(const twins::Widget* pWgt, const twins::KeyCode &kc, twins::String &str, int16_t &cursorPos) { return false; }
    virtual void onCheckboxToggle(const twins::Widget* pWgt) {}
    virtual void onPageControlPageChange(const twins::Widget* pWgt, uint8_t newPageIdx) {}
    virtual void onListBoxSelect(const twins::Widget* pWgt, int16_t selIdx) {}
    virtual void onListBoxChange(const twins::Widget* pWgt, int16_t newIdx) {}
    virtual void onRadioSelect(const twins::Widget* pWgt) {}
    virtual void onCustomWidgetDraw(const twins::Widget* pWgt) {}
    virtual bool onCustomWidgetInputEvt(const twins::Widget* pWgt, const twins::KeyCode &kc) { return false; }
    virtual bool onWindowUnhandledInputEvt(const twins::Widget* pWgt, const twins::KeyCode &kc) { return false; }
    // common state queries
    virtual bool isEnabled(const twins::Widget*) { return true; }
    virtual bool isFocused(const twins::Widget*) { return false; }
    virtual bool isVisible(const twins::Widget*) { return true; }
    virtual twins::WID& getFocusedID() = 0;
    // widget-specific queries
    virtual void getWindowCoord(const twins::Widget*, twins::Coord &coord) {}
    virtual void getWindowTitle(const twins::Widget*, twins::String &title) {}
    virtual bool getCheckboxChecked(const twins::Widget*) { return false; }
    virtual void getLabelText(const twins::Widget*, twins::String &out) {}
    virtual void getEditText(const twins::Widget*, twins::String &out) {}
    virtual bool getLedLit(const twins::Widget*) { return false; }
    virtual void getLedText(const twins::Widget*, twins::String &out) {}
    virtual void getProgressBarState(const twins::Widget*, int &pos, int &max) {}
    virtual int  getPageCtrlPageIndex(const twins::Widget*) { return 0; }
    virtual void getListBoxState(const twins::Widget*, int16_t &itemIdx, int16_t &selIdx, int16_t &itemsCount) {}
    virtual void getListBoxItem(const twins::Widget*, int itemIdx, twins::String &out) {}
    virtual int  getRadioIndex(const twins::Widget*) { return -1; }
    virtual void getTextBoxLines(const twins::Widget*, const twins::Vector<twins::StringRange> **ppLines, bool &changed) {}
    // requests
    virtual void invalidate(twins::WID id, bool instantly = false) {}
};

// -----------------------------------------------------------------------------