/******************************************************************************
 * @brief   TWins - demo window definition
 * @author  Mariusz Midor
 *****************************************************************************/

#pragma once

#include "twins.hpp"

// -----------------------------------------------------------------------------

/** Main window widgets unique id's */
enum WndMainIDs
{
    ID_INVALID, // value 0 is reserved to WIDGET_ID_NONE
    ID_WND,
        ID_BTN_TOASTER,
        ID_PGCONTROL,
            ID_PAGE_VER,
                ID_PANEL_VERSIONS,
                    ID_LABEL_FW_VERSION,
                    ID_LABEL_DATE,
                    ID_LABEL_ABOUT,
                ID_PANEL_STATE,
                    ID_LED_PUMP,
                    ID_LED_LOCK,
                    ID_LED_BATTERY,
                ID_PANEL_KEY,
                    ID_LABEL_KEYSEQ,
                    ID_LABEL_KEYNAME,
                ID_CHBX_ENBL,
                ID_CHBX_LOCK,
                ID_BTN_YES,
                ID_BTN_NO,
                ID_BTN_POPUP,
                ID_PRGBAR_1,
                ID_PRGBAR_2,
                ID_PRGBAR_3,
            ID_PAGE_SERV,
                ID_LAYER_1,
                    ID_LABEL_MULTI_FMT,
                    ID_LISTBOX,
                ID_LAYER_2,
                    ID_RADIO_1,
                    ID_RADIO_2,
                    ID_RADIO_3,
                ID_CHBX_L1,
                ID_CHBX_L2,
            ID_PAGE_DIAG,
                ID_PANEL_EDT,
                    ID_EDT_1,
                    ID_EDT_2,
                ID_CUSTOMWGT1,
                ID_PANEL_CHBX,
                    ID_LBL_CHBXTITLE,
                    ID_CHBX_A,
                    ID_CHBX_B,
                    ID_CHBX_C,
                    ID_CHBX_D,
            ID_PAGE_INACTIV,
                ID_PANEL_EMPTY_1,
                    ID_LBL_WORDWRAP,
                    ID_BTN_NOACTION,
                ID_PANEL_EMPTY_2,
                    ID_LBL_EMPTY_2,
            ID_PAGE_TEXTBOX,
                ID_TBX_LOREMIPSUM,
                ID_TBX_1LINE,
            ID_PAGE_COMBOBOX,
                ID_CBX_OPTIONS,
                ID_CBX_COLORS,
                ID_LBX_UNDEROPTIONS,
                ID_BTN_SAYYES,
                ID_BTN_SAYNO,
                ID_BTN_1P5,
        ID_LABEL_FTR,
};


// popup window IDs
enum WndPopupIDs
{
    IDPP_INVALID,
    IDPP_WND,
        IDPP_LBL_MSG,
        IDPP_BTN_YES,
        IDPP_BTN_NO,
        IDPP_BTN_CANCEL,
        IDPP_BTN_OK,
};

// -----------------------------------------------------------------------------

extern twins::IWindowState * getWndMain();
extern const twins::Widget * pWndMainWidgets;
extern const uint16_t wndMainNumPages;

extern twins::IWindowState * getWndPopup();
extern const twins::Widget * pWndPopupWidgets;
