# Change log

# 1.0.6
### Updates
- The user can no longer choose if he wants to use a custom title bar or not.
- Changed `MOVE_WINDOW` hot key from `Ctrl + F1` to `Alt + A`.
- Overlay window position and size can be set from settings (new overlay settings page).
    - The resizing needs to be worked on to allow resize from all directions.
- Updated NumberBlock to set the number of decimals desired.
- Re-added `update_notes.json` for i18ed update notes.

### Fixes
- Finished translating settings page.

### Todo
- [ ] Show/hide audio sessions.
- [X] Re-implement I18Ned "What's new" page.
- [ ] Enable the user to change hot key key, not only modifiers.
- [ ] Add more hot keys (audio session control, reload, restart, show on current screen, PiP).
    - [ ] Reload.
    - [ ] Restart.
    - [X] Show on current screen.
    - [ ] Enable/disable PiP.
    - [ ] Enable/disable overlay mode.
- [ ] Translate all strings.
    - [X] Settings page.
    - [X] Main window menu.
    - [ ] Audio profiles pages.
    - [ ] Audio sessions page.
    - [ ] Overlay page.
- [ ] Move audio controls to AudioViewer.


## 1.0.5
### Updates
- Added overlay mode, unresizable window with no close/minimize/maximize button.
- Added indicators to show how many audio sessions the window can currently host (on the right and on the top).

### Fixes
- Fixed "Show inactive audio sessions on startup" being defaulted to false (when starting the app for the first time). It is now defaulted to true.

### Todo
- [ ] Show/hide audio sessions.
- [ ] Re-implement I18Ned "What's new" page.
- [ ] Enable the user to change hot key key, not only modifiers.
- [ ] Add more hot keys (audio session control, reload, restart, show on current screen, PiP).
- [ ] Translate all strings.



## 1.0.4
### Updates
- Added overlay mode, unresizable window with no close/minimize/maximize button.
- Updated hot keys so that their modifier can be modified by the user. The user will be able to edit the key in future updates.
- Adding new translations to all menus, starting with the main window.
- Translations added to other menus.
- Added indicators to show how many audio sessions the window can currently host (on the right and on the top).

### Fixes

### Bugs
- Audio sessions will need to be reloaded manually after the computer has been put to sleep or hibernation.
- On some computers, pluggin/unplugging an audio jack will crash the app. The issue seems to be coming from the audio driver application.

### Todo
- [x] Disable/enable hot keys.
- [ ] Show/hide audio sessions.
- [ ] Re-implement I18Ned "What's new" page.
- [ ] Enable the user to change hot key key, not only modifiers.
- [ ] Add more hot keys (audio session control, reload, restart, show on current screen, PiP).
- [ ] Translate all strings.