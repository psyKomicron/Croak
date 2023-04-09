# *Change log*

# 1.2.1
## Updates
- Added background color (in `OverlaySettingsPage`) to see the size of the screen.

## Fixes

## Bugs
- Audio sessions will need to be reloaded manually after the computer has been put to sleep or hibernation.
- On some computers, pluggin/unplugging an audio jack will crash the app. The issue seems to be coming from the audio driver application.
- Some I18Ned applications don't have a real name: "ms-resource:AppStoreName".

## Todo
- [ ] Show/hide audio sessions.
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
- [ ] Change app logo to be the 'audio mixer' font icon.
- [ ] Handle `ms-resources` when reading package information from app manifests.
- [ ] Invalidate and release all audio objects when bug #2 occurs. This will allow the application to reload audio sessions without having to restart.
- [ ] Implement `AudioProfileEditPage` exit confirmation dialog (changes check not done).
- [ ] Fix overlay settings not properly working.


# 1.2.0
*Version bump from 1.0.x to 1.2.x is caused by GitHub branch name being called 1.1.x and package version not following that.*
## Updates
- Moving all audio related logic from MainWindow to AudioViewer (no regression allowed).
- Split user profile in 2:
    - UserProfile to store window properties, overlay settings and stuff like that.
    - AudioProfile to store all audio related settings.
*-> this change was needed to have more flexibility and to clearly split responsabilities between `MainWindow` and `AudioViewer`.*
- Added `UserProfile` to save window and app related settings. `AudioProfile` now handles only audio related settings.
- Removed app/window related settings from `AudioProfile` (ie `IsAlwaysOnTop`, `WindowDisplayRect`).
- Moved MessageBar to the bottom to replicate Youtube's mobile app notifications (and updating animations accordingly).
- Updated `ToggleButton` to use `IInspectable` (to display generic content) for `OnIcon` and `OffIcon`. It might be changed back to `hstring` since icon composition is not that useful.
- `OverlaySettingsPage` now adds itself to the navigation breadcrumbs.

## Fixes
- Fixed audio sessions being added even if another session with the same grouping parameter.
- Fixed `Audio session volume icon is not green when the audio session is active in vertical layout.`
- Fixed `ToggleIconButton` not properly behaving on pointer over by setting its background.

## Bugs
- Audio sessions will need to be reloaded manually after the computer has been put to sleep or hibernation.
- On some computers, pluggin/unplugging an audio jack will crash the app. The issue seems to be coming from the audio driver application.
- Some I18Ned applications don't have a real name: "ms-resource:AppStoreName".

## Todo
- [ ] Show/hide audio sessions.
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
- [ ] Change app logo to be the 'audio mixer' font icon.
- [ ] Handle `ms-resources` when reading package information from app manifests.
- [ ] Invalidate and release all audio objects when bug #2 occurs. This will allow the application to reload audio sessions without having to restart.

*This update*
- [X] Move audio controls to `AudioViewer`.
- [X] Create UserProfile and store in it `AudioProfile` so that the audio settings are separated from other settings (such as window layout, overlay position and future stuff).
- [ ] Implement `AudioProfileEditPage` exit confirmation dialog (changes check not done).
- [X] Get settings from profiles:
    - From audio profile.
    - From user profile.



# 1.0.6
## Updates
- The user can no longer choose if he wants to use a custom title bar or not.
- Changed `MOVE_WINDOW` hot key from `Ctrl + F1` to `Alt + A`.
- Overlay window position and size can be set from settings (new overlay settings page).
    - The resizing needs to be worked on to allow resize from all directions.
- Updated NumberBlock to set the number of decimals desired.
- Re-added `update_notes.json` for i18ed update notes.

## Fixes
- Finished translating settings page.

## Bugs
- Audio sessions will need to be reloaded manually after the computer has been put to sleep or hibernation.
- On some computers, pluggin/unplugging an audio jack will crash the app. The issue seems to be coming from the audio driver application.

## Todo
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



# 1.0.5
## Updates
- Added overlay mode, unresizable window with no close/minimize/maximize button.
- Added indicators to show how many audio sessions the window can currently host (on the right and on the top).

## Fixes
- Fixed "Show inactive audio sessions on startup" being defaulted to false (when starting the app for the first time). It is now defaulted to true.

## Bugs
- Audio sessions will need to be reloaded manually after the computer has been put to sleep or hibernation.
- On some computers, pluggin/unplugging an audio jack will crash the app. The issue seems to be coming from the audio driver application.

## Todo
- [ ] Show/hide audio sessions.
- [ ] Re-implement I18Ned "What's new" page.
- [ ] Enable the user to change hot key key, not only modifiers.
- [ ] Add more hot keys (audio session control, reload, restart, show on current screen, PiP).
- [ ] Translate all strings.



# 1.0.4
## Updates
- Added overlay mode, unresizable window with no close/minimize/maximize button.
- Updated hot keys so that their modifier can be modified by the user. The user will be able to edit the key in future updates.
- Adding new translations to all menus, starting with the main window.
- Translations added to other menus.
- Added indicators to show how many audio sessions the window can currently host (on the right and on the top).

## Fixes

## Bugs
- Audio sessions will need to be reloaded manually after the computer has been put to sleep or hibernation.
- On some computers, pluggin/unplugging an audio jack will crash the app. The issue seems to be coming from the audio driver application.

## Todo
- [x] Disable/enable hot keys.
- [ ] Show/hide audio sessions.
- [ ] Re-implement I18Ned "What's new" page.
- [ ] Enable the user to change hot key key, not only modifiers.
- [ ] Add more hot keys (audio session control, reload, restart, show on current screen, PiP).
- [ ] Translate all strings.