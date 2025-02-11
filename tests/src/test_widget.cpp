/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins_transform_window.hpp"
#include "twins.hpp"
#include "twins_utils.hpp"
#include "twins_window_mngr.hpp"
#include "../../lib/src/twins_widget_prv.hpp"

// -----------------------------------------------------------------------------

enum WndTestIDs
{
    ID_INVALID,
    ID_WND,
        ID_PGCTRL,
            ID_PAGE1,
                ID_LBL1,
                ID_LBL2,
                ID_BTN1,
                ID_BTN2,
                ID_BTN3,
                ID_LED,
            ID_PAGE2,
        ID_PANEL,
        ID_EDIT,
        ID_RADIO,
        ID_CHECK,
        ID_PROGRESS,
        ID_LISTBOX,
        ID_TEXTBOX,
        ID_TEXTBOX_EMPTY,
        ID_COMBOBOX,
};

class WindowTestState : public twins::IWindowState
{
public:
    void init(const twins::Widget *pWindowWgts) override
    {
        mpWgts = pWindowWgts;
    }

    const twins::Widget *getWidgets() const override { return mpWgts; }

    twins::WID& getFocusedID() override { return wgtId; };

    void getLabelText(const twins::Widget*, twins::String &out) override
    {
        out = "Label 1" "\n" "..but Line 2";
    }

    void getListBoxState(const twins::Widget*, int16_t &itemIdx, int16_t &selIdx, int16_t &itemsCount) override
    {
        itemIdx = 1;
        selIdx = 0;
        itemsCount = 3;
    }

    void getListBoxItem(const twins::Widget*, int itemIdx, twins::String &out) override
    {
        out.appendFmt("item: %d", itemIdx);
    }

    void getComboBoxState(const twins::Widget* /* pWgt */, int16_t &itemIdx, int16_t &selIdx, int16_t &itemsCount, bool &dropDown) override
    {
        itemIdx = 1;
        selIdx = 0;
        itemsCount = 8;
        dropDown = true;
    }

    void getComboBoxItem(const twins::Widget* /* pWgt */, int itemIdx, twins::String &out) override
    {
        out.appendFmt("item: %d", itemIdx);
    }

    void getTextBoxState(const twins::Widget* pWgt, const twins::Vector<twins::CStrView> **ppLines, int16_t &topLine) override
    {
        if (pWgt->id == ID_TEXTBOX_EMPTY)
        {
            wrapString.updateLines();
            *ppLines = &wrapString.getLines();
            topLine = 5;
            return;
        }

        wrapString = "Lorem ipsum \e[1m dolor \e[0m sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";

        wrapString.config(15);
        *ppLines = &wrapString.getLines();
        // force scanning of invisible lines:
        topLine = 2;
    }

    void onPageControlPageChange(const twins::Widget* /* pWgt */, uint8_t newPageIdx) override
    {
        pgIndex = newPageIdx;
    }

    int getPageCtrlPageIndex(const twins::Widget* /* pWgt */) override
    {
        return pgIndex;
    }

    void onButtonClick(const twins::Widget* pWgt, const twins::KeyCode &/* kc */) override
    {
        clickedId = pWgt->id;
    }

    void onCheckboxToggle(const twins::Widget* /* pWgt */) override
    {
        chbxChecked = !chbxChecked;
    }

    bool getCheckboxChecked(const twins::Widget* /* pWgt */) override
    {
        return chbxChecked;
    }

public:
    const twins::Widget *mpWgts = nullptr;
    twins::WID wgtId = {};
    twins::WID clickedId = {};
    twins::util::WrappedString wrapString;
    uint8_t pgIndex = 0;
    bool chbxChecked = {};
};


static WindowTestState wndTest;
twins::IWindowState * getWndTest();


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static constexpr twins::Widget wndTestDef =
{
    type    : twins::Widget::Window,
    id      : ID_WND,
    coord   : { 5, 5 },
    size    : { 100, 50 },
    { window : {
        title       : "**Test**Window**",
        fgColor     : {},
        bgColor     : {},
        isPopup     : true, // draw shadow
        getState    : getWndTest,
    }},
    link    : { (const twins::Widget[])
    {
        {
            type    : twins::Widget::PageCtrl,
            id      : ID_PGCTRL,
            coord   : { 2, 2 },
            size    : { 80, 40 },
            { pagectrl : {
                tabWidth    : 20,
            }},
            link    : { (const twins::Widget[])
            {
                {
                    type    : twins::Widget::Page,
                    id      : ID_PAGE1,
                    coord   : { 2, 2 },
                    size    : { },
                    { page : {
                        title   : "Page title",
                        fgColor : {},
                    }},
                    link    : { (const twins::Widget[])
                    {
                        {
                            type    : twins::Widget::Label,
                            id      : ID_LBL1,
                            coord   : { 2, 2 },
                            size    : { 30, 4 },
                            { label : {
                                text    : {},
                                fgColor : {},
                                bgColor : {},
                            }}
                        },
                        {
                            type    : twins::Widget::Label,
                            id      : ID_LBL2,
                            coord   : { 2, 2 },
                            size    : { 30, 4 },
                            { label : {
                                text    : "Label: ",
                                fgColor : {},
                                bgColor : {},
                            }}
                        },
                        {
                            type    : twins::Widget::Button,
                            id      : ID_BTN1,
                            coord   : { 5, 7 },
                            size    : {},
                            { button : {
                                text    : "YES",
                                fgColor : {},
                                bgColor : {},
                                style   : twins::ButtonStyle::Solid
                            }}
                        },
                        {
                            type    : twins::Widget::Button,
                            id      : ID_BTN2,
                            coord   : { 5, 8 },
                            size    : {},
                            { button : {
                                text    : "NO",
                                fgColor : {},
                                bgColor : {},
                                style   : twins::ButtonStyle::Simple
                            }}
                        },
                        {
                            type    : twins::Widget::Button,
                            id      : ID_BTN3,
                            coord   : { 15, 8 },
                            size    : {},
                            { button : {
                                text    : "1.5 Line",
                                fgColor : {},
                                bgColor : {},
                                style   : twins::ButtonStyle::Solid1p5
                            }}
                        },
                        {
                            type    : twins::Widget::Led,
                            id      : ID_LED,
                            coord   : { 5, 9 },
                            size    : {},
                            { led : {
                                text        : "ENABLED",
                                fgColor     : {},
                                bgColorOff  : {},
                                bgColorOn   : {},
                            }}
                        },
                        { /* NUL */ }
                    }}
                },
                {
                    type    : twins::Widget::Page,
                    id      : ID_PAGE2,
                    coord   : { 2, 2 },
                    size    : { },
                    { page : {
                        title   : "Page 2 title",
                        fgColor : {},
                    }},
                    link    : { (const twins::Widget[])
                    {
                        { /* NUL */ }
                    }}
                },
                { /* NUL */ }
            }}
        },
        {
            type    : twins::Widget::Panel,
            id      : ID_PANEL,
            coord   : { 2, 30 },
            size    : { 30, 4 },
            { panel : {
                title   : "Panel",
                fgColor : {},
                bgColor : {},
                noFrame : {}
            }}
        },
        {
            type    : twins::Widget::TextEdit,
            id      : ID_EDIT,
            coord   : { 2, 30 },
            size    : { 30, 4 },
            { textedit : {
                fgColor : {},
                bgColor : {},
            }}
        },
        {
            type    : twins::Widget::Radio,
            id      : ID_RADIO,
            coord   : { 2, 32 },
            size    : { 10, 1 },
            { radio : {
                text    : "Option 1",
                fgColor : {},
                groupId : 1,
                radioId : 1,
            }}
        },
        {
            type    : twins::Widget::CheckBox,
            id      : ID_CHECK,
            coord   : { 2, 34 },
            size    : { 10, 1 },
            { checkbox : {
                text    : "radio",
                fgColor : {},
            }}
        },
        {
            type    : twins::Widget::ProgressBar,
            id      : ID_PROGRESS,
            coord   : { 2, 34 },
            size    : { 10, 1 },
            { progressbar : {
                fgColor : {},
                style   : twins::PgBarStyle::Hash
            }}
        },
        {
            type    : twins::Widget::ListBox,
            id      : ID_LISTBOX,
            coord   : { 80, 2 },
            size    : { 10, 10 },
            { listbox : {
                fgColor : {},
                bgColor : {},
                noFrame : false
            }}
        },
        {
            type    : twins::Widget::TextBox,
            id      : ID_TEXTBOX,
            coord   : { 80, 20 },
            size    : { 10, 10 },
            { textbox : {
                fgColor : {},
                bgColor : {},
            }}
        },
        {
            type    : twins::Widget::TextBox,
            id      : ID_TEXTBOX_EMPTY,
            coord   : { 80, 20 },
            size    : { 10, 10 },
            { textbox : {
                fgColor : {},
                bgColor : {},
            }}
        },
        {
            type    : twins::Widget::ComboBox,
            id      : ID_COMBOBOX,
            coord   : { 10, 10 },
            size    : { 10, 1 },
            { combobox : {
                fgColor : {},
                bgColor : {},
                dropDownSize : 5
            }}
        },
        { /* NUL */ }
    }}
};

#pragma GCC diagnostic pop // ignored "-Wmissing-field-initializers"


constexpr auto wndTestWidgets = twins::transforWindowDefinition<&wndTestDef>();
const twins::Widget * pWndTestWidgets = wndTestWidgets.begin();

twins::IWindowState * getWndTest()
{
    wndTest.init(pWndTestWidgets);
    return &wndTest;
}

// -----------------------------------------------------------------------------

class WIDGET : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        twins::flushBuffer();
    }
};

// -----------------------------------------------------------------------------

TEST_F(WIDGET, isPointIn)
{
    twins::Rect r;
    r.coord = {2, 1};
    r.size = {10, 5};

    // x < left
    EXPECT_FALSE(twins::isPointWithin(1, 1, r));
    // x >= right
    EXPECT_FALSE(twins::isPointWithin(r.coord.col + r.size.width, 1, r));
    // y < top
    EXPECT_FALSE(twins::isPointWithin(2, 0, r));
    // y >= bottom
    EXPECT_FALSE(twins::isPointWithin(2, r.coord.row + r.size.height, r));

    // x == left, y == top
    EXPECT_TRUE(twins::isPointWithin(2, 1, r));
    // x = right-1, y = bottom-1
    EXPECT_TRUE(twins::isPointWithin(r.coord.col + r.size.width - 1, r.coord.row + r.size.height - 1, r));


    // rect is empty
    r.size = {0, 0};
    EXPECT_FALSE(twins::isPointWithin(2, 1, r));
}

TEST_F(WIDGET, isRectIn)
{
    const twins::Rect e = { {2, 1}, {10, 5}};
    const twins::Rect i = e;

    // internal is the same size as external
    EXPECT_TRUE(twins::isRectWithin(i, e));
    // internal is smaller and do not touch external
    EXPECT_TRUE(twins::isRectWithin({ {3, 2}, {8, 3}}, e));

    // l,t corner within, r,b outside
    EXPECT_FALSE(twins::isRectWithin(i + twins::Size{1,1}, e));

    // internal excess to the left
    EXPECT_FALSE(twins::isRectWithin(i - twins::Coord{1,0}, e));
    // internal excess to the right
    EXPECT_FALSE(twins::isRectWithin(i + twins::Coord{1,0}, e));
    // internal excess to the top
    EXPECT_FALSE(twins::isRectWithin(i - twins::Coord{0,1}, e));
    // internal excess to the bottom
    EXPECT_FALSE(twins::isRectWithin(i + twins::Coord{0,1}, e));
}

TEST_F(WIDGET, drawWidget)
{
    twins::drawWidget(pWndTestWidgets, ID_TEXTBOX);

    // draw pressed button
    twins::g_ws.pMouseDownWgt = twins::getWidget(pWndTestWidgets, ID_BTN1);
    twins::drawWidget(pWndTestWidgets, ID_BTN1);
    twins::g_ws.pMouseDownWgt = {};

    // draw all
    auto t = twins::pPAL->getTimeStamp();
    twins::drawWidget(pWndTestWidgets);
    twins::writeChar('\n', 3);

    t = twins::pPAL->getTimeDiff(t);
    TWINS_LOG_D("Drawn in %u ms", t);
}

TEST_F(WIDGET, drawWidgets)
{
    twins::WID wids[] = { ID_CHECK, ID_PANEL };
    twins::drawWidgets(pWndTestWidgets, wids);
}

TEST_F(WIDGET, wndManager)
{
    twins::WndManager wmngr;

    EXPECT_EQ(0, wmngr.size());
    EXPECT_EQ(nullptr, wmngr.topWndWidgets());
    EXPECT_FALSE(wmngr.visible(getWndTest()));

    wmngr.show(nullptr);
    wmngr.show(getWndTest());
    wmngr.show(getWndTest());
    EXPECT_EQ(1, wmngr.size());
    EXPECT_TRUE(wmngr.visible(getWndTest()));
    EXPECT_EQ(getWndTest(), wmngr.topWnd());

    wmngr.redrawAll();
    wmngr.hide(nullptr);
    wmngr.hide(getWndTest());
    wmngr.hide(getWndTest());
    EXPECT_EQ(0, wmngr.size());
}

TEST_F(WIDGET, toString)
{
    for (int wgt = twins::Widget::None; wgt < twins::Widget::_Count; wgt++)
    {
        const char *wname = twins::toString(twins::Widget::Type(wgt));
        EXPECT_TRUE(wname);
        EXPECT_STRNE(wname, "");
    }

    const char *wname = twins::toString(twins::Widget::Type(-1));
    EXPECT_STREQ(wname, "?");
}

TEST_F(WIDGET, getScreenCoord)
{
    const auto *p_wnd = wndTest.getWidgets();
    ASSERT_NE(nullptr, p_wnd);
    ASSERT_EQ(twins::Widget::Window, p_wnd->type);
    const auto *p_lbl = twins::getWidget(p_wnd, ID_LED);
    ASSERT_NE(nullptr, p_lbl);
    ASSERT_EQ(twins::Widget::Led, p_lbl->type);

    {
        twins::resetInternalState();
        auto c = twins::getScreenCoord(p_lbl);
        EXPECT_EQ(34, c.col);
        EXPECT_EQ(18, c.row);
    }

    {
        twins::resetInternalState();
        auto c = twins::getScreenCoord(p_wnd);
        EXPECT_EQ(5, c.col);
        EXPECT_EQ(5, c.row);
    }
}

TEST_F(WIDGET, pageControl)
{
    const auto *p_wnd = wndTest.getWidgets();
    ASSERT_NE(nullptr, p_wnd);
    const auto *p_pgctrl = twins::getWidget(p_wnd, ID_PGCTRL);

    {
        auto id = twins::wgt::getPageID(p_pgctrl, 0);
        EXPECT_EQ(ID_PAGE1, id);

        id = twins::wgt::getPageID(p_pgctrl, 123);
        EXPECT_EQ(twins::WIDGET_ID_NONE, id);

        id = twins::wgt::getPageID(p_pgctrl, -3);
        EXPECT_EQ(twins::WIDGET_ID_NONE, id);
    }

    {
        auto idx = twins::wgt::getPageIdx(p_pgctrl, ID_PAGE2);
        EXPECT_EQ(1, idx);

        idx = twins::wgt::getPageIdx(p_pgctrl, 123);
        EXPECT_EQ(-1, idx);
    }

    {
        wndTest.pgIndex = 0;
        twins::wgt::selectNextPage(p_wnd, ID_PGCTRL, true);
        EXPECT_EQ(1, wndTest.pgIndex);
        twins::wgt::selectNextPage(p_wnd, ID_PGCTRL, true);
        EXPECT_EQ(0, wndTest.pgIndex);
        twins::wgt::selectNextPage(p_wnd, ID_PGCTRL, false);
        EXPECT_EQ(1, wndTest.pgIndex);
        twins::wgt::selectNextPage(p_wnd, ID_PGCTRL, false);
        EXPECT_EQ(0, wndTest.pgIndex);

        wndTest.pgIndex = 0;
        twins::wgt::selectPage(p_wnd, ID_PGCTRL, ID_PAGE2);
        EXPECT_EQ(1, wndTest.pgIndex);
    }
}

TEST_F(WIDGET, isWidgetVisible)
{
    const auto *p_wnd = wndTest.getWidgets();
    ASSERT_NE(nullptr, p_wnd);
    const auto *p_btn = twins::getWidget(p_wnd, ID_BTN1);

    EXPECT_TRUE(twins::isWidgetVisible(p_wnd, p_btn));
}

TEST_F(WIDGET, isWidgetEnabled)
{
    const auto *p_wnd = wndTest.getWidgets();
    ASSERT_NE(nullptr, p_wnd);
    const auto *p_btn = twins::getWidget(p_wnd, ID_BTN1);

    EXPECT_TRUE(twins::isWidgetEnabled(p_wnd, p_btn));
}

TEST_F(WIDGET, processInput_Key)
{
    const auto *p_wnd = wndTest.getWidgets();
    ASSERT_NE(nullptr, p_wnd);

    {
        twins::KeyCode kc = {};
        twins::processInput(p_wnd, kc);
    }

    {
        twins::KeyCode kc = {};
        kc.key = twins::Key::Esc;
        kc.m_spec = true;
        twins::processInput(p_wnd, kc);
    }

    {
        twins::KeyCode kc = {};
        kc.key = twins::Key::Tab;
        kc.m_spec = true;
        twins::processInput(p_wnd, kc);
    }

    {
        twins::KeyCode kc = {};
        kc.key = twins::Key::Home;
        kc.m_spec = true;
        twins::processInput(p_wnd, kc);
    }
}

TEST_F(WIDGET, processInput_OnWidget)
{
    const auto *p_wnd = wndTest.getWidgets();
    ASSERT_NE(nullptr, p_wnd);

    {
        wndTest.wgtId = ID_PGCTRL;
        wndTest.pgIndex = 0;
        twins::KeyCode kc = {};
        kc.m_spec = true;

        kc.key = twins::Key::PgDown;
        twins::processInput(p_wnd, kc);
        EXPECT_EQ(1, wndTest.pgIndex);

        kc.key = twins::Key::PgUp;
        twins::processInput(p_wnd, kc);
        EXPECT_EQ(0, wndTest.pgIndex);

        kc.key = twins::Key::F1;
        twins::processInput(p_wnd, kc);
        EXPECT_EQ(0, wndTest.pgIndex);
    }

    {
        wndTest.wgtId = ID_CHECK;
        wndTest.chbxChecked = false;
        twins::KeyCode kc = {};
        kc.m_spec = true;

        kc.key = twins::Key::Enter;
        twins::processInput(p_wnd, kc);
        EXPECT_TRUE(wndTest.chbxChecked);

        kc.key = twins::Key::Enter;
        twins::processInput(p_wnd, kc);
        EXPECT_FALSE(wndTest.chbxChecked);

        kc.key = twins::Key::F1;
        twins::processInput(p_wnd, kc);
        EXPECT_FALSE(wndTest.chbxChecked);
    }

    {
        wndTest.wgtId = ID_BTN1;
        wndTest.clickedId = {};
        twins::KeyCode kc = {};
        kc.m_spec = true;

        kc.key = twins::Key::Enter;
        twins::processInput(p_wnd, kc);
        EXPECT_EQ(ID_BTN1, wndTest.clickedId);

        wndTest.clickedId = {};
        kc.key = twins::Key::Enter;
        twins::processInput(p_wnd, kc);
        EXPECT_EQ(ID_BTN1, wndTest.clickedId);

        wndTest.clickedId = {};
        kc.key = twins::Key::F1;
        twins::processInput(p_wnd, kc);
        EXPECT_EQ(0, wndTest.clickedId);
    }
}

TEST_F(WIDGET, processInput_Mouse_BtnClick)
{
    const auto *p_wnd = wndTest.getWidgets();
    ASSERT_NE(nullptr, p_wnd);

    {
        // outside window
        twins::KeyCode kc = {};
        kc.key = twins::Key::MouseEvent;
        kc.mouse.btn = twins::MouseBtn::ButtonLeft;
        twins::processInput(p_wnd, kc);
    }

    {
        // button click
        const auto *p_btn = twins::getWidget(p_wnd, ID_BTN1);
        auto btn_coord = twins::getScreenCoord(p_btn);

        wndTest.clickedId = {};
        twins::KeyCode kc = {};
        kc.key = twins::Key::MouseEvent;
        kc.mouse.btn = twins::MouseBtn::ButtonLeft;
        kc.mouse.col = btn_coord.col;
        kc.mouse.row = btn_coord.row;
        twins::processInput(p_wnd, kc);

        kc.mouse.btn = twins::MouseBtn::ButtonReleased;
        twins::processInput(p_wnd, kc);
        EXPECT_EQ(ID_BTN1, wndTest.clickedId);
    }
}
