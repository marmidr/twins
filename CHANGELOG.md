# Changelog

See the [keepachangelog.com description](https://keepachangelog.com/en/1.0.0/).

## 0.27.0 - 2026-03-20

* Added
  * twins::SecurePassw
* Changed
  * CLI uses SecurePassw instead of raw strings
  * CLI history with public last item index

## 0.26.0 - 2025-06-26

* Added
  * twins::findCoveringWidget() - now the WndManager::hide() redraw only neccessary part of the underlying window
  * String::operator<()
  * Vector::insertionSort()
* Changed
  * WindowStateBase::invalidateImpl() updated - taken from demo, operates on invalidatedWgts

## 0.25.0 - 2025-06-11

* Added
  * IWindowState::onBeforeShow()
  * WindowStateBase::invalidatedWgts
  * Listbox - uses intense bg color when focused
* Fixed
  * Bug in WndManager::hide() - the TWins internal state must be reset to not bring
    the widgets edit-mode from popup to the underlying window

## 0.24.0 - 2025-05-26

* Added
  * String::replaceChar
  * String::trimBuff
  * TextBox - param scrollLines
  * WindowStateBase::forEachChild
* Fixed
  * Bug fix in the Scroolbar drawing code

## 0.23.0 - 2025-04-14

* Added
  * getWidgetBgColor()
  * getWidgetFgColor()
* Changed
  * ComboBox uses intensified BG color, like TextEdit
  * CheckBox, RadioButton - cursor default pos does not shadow the check state
  * ComboBox, TextBox - cursor default position changed
* Fixed
  * PageControl - restore pages bg color after page is drawn
  * List/ComboBox - corrected width when items < height

## 0.22.0 - 2025-03-31

* Added
  * String::strip()
* Fixed
  * CLI: help [cmd] works for commands with aliases "ver|V"

## 0.21.0 - 2025-03-13

* Changed
  * CLI: each command may be individually password-protected, with a different access level

## 0.20.0 - 2025-02-14

* Added
  * logRawWriteLen()
  * CLI: multiple password mode added
* Changed
  * new approach to colors themes
* Removed
  * glob::extern twins::IPal& pal;

## 0.19.3 - 2025-02-11

* Added
  * CHANGELOG
  * Demo: tracking the widgets drawer
* Changed
  * tests moved outside the `lib/` folder
  * BitBucket CI updated
* Deprecated
* Removed
* Fixed
