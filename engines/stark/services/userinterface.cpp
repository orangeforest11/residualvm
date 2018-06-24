/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/events.h"
#include "common/stream.h"
#include "common/system.h"

#include "engines/stark/services/userinterface.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/services/gameinterface.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/resourceprovider.h"

#include "engines/stark/ui/cursor.h"
#include "engines/stark/ui/menu/diaryindex.h"
#include "engines/stark/ui/menu/mainmenu.h"
#include "engines/stark/ui/menu/settingsmenu.h"
#include "engines/stark/ui/menu/saveloadmenu.h"
#include "engines/stark/ui/menu/fmvmenu.h"
#include "engines/stark/ui/menu/diarypages.h"
#include "engines/stark/ui/world/inventorywindow.h"
#include "engines/stark/ui/world/fmvscreen.h"
#include "engines/stark/ui/world/gamescreen.h"
#include "engines/stark/ui/world/gamewindow.h"

namespace Stark {

UserInterface::UserInterface(Gfx::Driver *gfx) :
		_gfx(gfx),
		_cursor(nullptr),
		_diaryIndexScreen(nullptr),
		_mainMenuScreen(nullptr),
		_settingsMenuScreen(nullptr),
		_saveMenuScreen(nullptr),
		_loadMenuScreen(nullptr),
		_fmvMenuScreen(nullptr),
		_diaryPagesScreen(nullptr),
		_exitGame(false),
		_fmvScreen(nullptr),
		_gameScreen(nullptr),
		_interactive(true),
		_interactionAttemptDenied(false),
		_currentScreen(nullptr),
		_gameWindowThumbnail(nullptr),
		_prevScreenNameStack() {
}

UserInterface::~UserInterface() {
	freeGameScreenThumbnail();

	delete _gameScreen;
	delete _fmvScreen;
	delete _diaryIndexScreen;
	delete _cursor;
	delete _mainMenuScreen;
	delete _settingsMenuScreen;
	delete _saveMenuScreen;
	delete _loadMenuScreen;
	delete _fmvMenuScreen;
	delete _diaryPagesScreen;
}

void UserInterface::init() {
	_cursor = new Cursor(_gfx);

	_mainMenuScreen = new MainMenuScreen(_gfx, _cursor);
	_gameScreen = new GameScreen(_gfx, _cursor);
	_diaryIndexScreen = new DiaryIndexScreen(_gfx, _cursor);
	_settingsMenuScreen = new SettingsMenuScreen(_gfx, _cursor);
	_saveMenuScreen = new SaveMenuScreen(_gfx, _cursor);
	_loadMenuScreen = new LoadMenuScreen(_gfx, _cursor);
	_fmvMenuScreen = new FMVMenuScreen(_gfx, _cursor);
	_diaryPagesScreen = new DiaryPagesScreen(_gfx, _cursor);
	_fmvScreen = new FMVScreen(_gfx, _cursor);

	_prevScreenNameStack.push(Screen::kScreenMainMenu);
	_currentScreen = _fmvScreen;

	// Play the FunCom logo video
	_fmvScreen->play("1402.bbb");
}

void UserInterface::update() {
	StarkStaticProvider->onGameLoop();

	// Check for UI mouse overs
	_currentScreen->handleMouseMove();
}

void UserInterface::handleMouseMove(const Common::Point &pos) {
	_cursor->setMousePosition(pos);
}

void UserInterface::handleMouseUp() {
	// Only the settings menu needs to handle this event
	_settingsMenuScreen->handleMouseUp();
}

void UserInterface::handleClick() {
	_currentScreen->handleClick();
}

void UserInterface::handleRightClick() {
	_currentScreen->handleRightClick();
}

void UserInterface::handleDoubleClick() {
	_currentScreen->handleDoubleClick();
}

void UserInterface::inventoryOpen(bool open) {
	// Make the inventory update its contents.
	if (open) {
		_gameScreen->getInventoryWindow()->open();
	} else {
		_gameScreen->getInventoryWindow()->close();
	}
}

int16 UserInterface::getSelectedInventoryItem() const {
	if (_gameScreen) {
		return _gameScreen->getInventoryWindow()->getSelectedInventoryItem();
	} else {
		return -1;
	}
}

void UserInterface::selectInventoryItem(int16 itemIndex) {
	_gameScreen->getInventoryWindow()->setSelectedInventoryItem(itemIndex);
}

void UserInterface::requestFMVPlayback(const Common::String &name) {
	changeScreen(Screen::kScreenFMV);

	_fmvScreen->play(name);
}

void UserInterface::onFMVStopped() {
	backPrevScreen();
}

void UserInterface::changeScreen(Screen::Name screenName) {
	if (screenName == _currentScreen->getName()) {
		return;
	}

	_prevScreenNameStack.push(_currentScreen->getName());
	_currentScreen->close();
	_currentScreen = getScreenByName(screenName);
	_currentScreen->open();
}

void UserInterface::backPrevScreen() {
	// No need to check the stack since at least there will be a MainMenuScreen in it
	// and MainMenuScreen will not request to go back
	changeScreen(_prevScreenNameStack.pop());

	// No need to push for going back
	_prevScreenNameStack.pop();
}

void UserInterface::quitToMainMenu() {
	changeScreen(Screen::kScreenGame);
	StarkResourceProvider->shutdown();
	changeScreen(Screen::kScreenMainMenu);
	_prevScreenNameStack.clear();
}

void UserInterface::restoreScreenHistory() {
	_prevScreenNameStack.clear();
	_prevScreenNameStack.push(Screen::kScreenMainMenu);
}

Screen *UserInterface::getScreenByName(Screen::Name screenName) const {
	switch (screenName) {
		case Screen::kScreenFMV:
			return _fmvScreen;
		case Screen::kScreenDiaryIndex:
			return _diaryIndexScreen;
		case Screen::kScreenGame:
			return _gameScreen;
		case Screen::kScreenMainMenu:
			return _mainMenuScreen;
		case Screen::kScreenSettingsMenu:
			return _settingsMenuScreen;
		case Screen::kScreenSaveMenu:
			return _saveMenuScreen;
		case Screen::kScreenLoadMenu:
			return _loadMenuScreen;
		case Screen::kScreenFMVMenu:
			return _fmvMenuScreen;
		case Screen::kScreenDiaryPages:
			return _diaryPagesScreen;
		default:
			error("Unhandled screen name '%d'", screenName);
	}
}

bool UserInterface::isInGameScreen() const {
	return _currentScreen->getName() == Screen::kScreenGame;
}

bool UserInterface::isInSaveLoadMenuScreen() const {
	Screen::Name name = _currentScreen->getName();
	return name == Screen::kScreenSaveMenu || name == Screen::kScreenLoadMenu;
}

bool UserInterface::isInventoryOpen() const {
	return _gameScreen->getInventoryWindow()->isVisible();
}

bool UserInterface::skipFMV() {
	if (_currentScreen->getName() == Screen::kScreenFMV) {
		_fmvScreen->stop();
		return true;
	}
	return false;
}

void UserInterface::render() {
	_currentScreen->render();

	// The cursor depends on the UI being done.
	if (_currentScreen->getName() != Screen::kScreenFMV) {
		_cursor->render();
	}
}

bool UserInterface::isInteractive() const {
	return _interactive;
}

void UserInterface::setInteractive(bool interactive) {
	if (interactive && !_interactive) {
		StarkGlobal->setNormalSpeed();
	} else if (!interactive && _interactive) {
		_interactionAttemptDenied = false;
	}

	_interactive = interactive;
}

void UserInterface::markInteractionDenied() {
	if (!_interactive) {
		_interactionAttemptDenied = true;
	}
}

bool UserInterface::wasInteractionDenied() const {
	return !_interactive && _interactionAttemptDenied;
}

void UserInterface::clearLocationDependentState() {
	_gameScreen->reset();
}

void UserInterface::optionsOpen() {
	changeScreen(Screen::kScreenDiaryIndex);
}

void UserInterface::saveGameScreenThumbnail() {
	freeGameScreenThumbnail();

	if (StarkGlobal->getLevel()) {
		// Re-render the screen to exclude the cursor
		StarkGfx->clearScreen();
		_gameScreen->render();
	}

	Graphics::Surface *big = _gameScreen->getGameWindow()->getScreenshot();
	assert(big->format.bytesPerPixel == 4);

	_gameWindowThumbnail = new Graphics::Surface();
	_gameWindowThumbnail->create(kThumbnailWidth, kThumbnailHeight, big->format);

	uint32 *dst = (uint32 *)_gameWindowThumbnail->getPixels();
	for (uint i = 0; i < _gameWindowThumbnail->h; i++) {
		for (uint j = 0; j < _gameWindowThumbnail->w; j++) {
			uint32 srcX = big->w * j / _gameWindowThumbnail->w;
			uint32 srcY = big->h * i / _gameWindowThumbnail->h;
			uint32 *src = (uint32 *)big->getBasePtr(srcX, srcY);

			// Copy RGBA pixel
			*dst++ = *src;
		}
	}

	big->free();
	delete big;
}

void UserInterface::freeGameScreenThumbnail() {
	if (_gameWindowThumbnail) {
		_gameWindowThumbnail->free();
		delete _gameWindowThumbnail;
		_gameWindowThumbnail = nullptr;
	}
}

const Graphics::Surface *UserInterface::getGameWindowThumbnail() const {
	return _gameWindowThumbnail;
}

void UserInterface::onScreenChanged() {
	_gameScreen->onScreenChanged();

	if (!isInGameScreen()) {
		_currentScreen->onScreenChanged();
	}
}

void UserInterface::notifyInventoryItemEnabled(uint16 itemIndex) {
	_gameScreen->notifyInventoryItemEnabled(itemIndex);
}

void UserInterface::notifyDiaryEntryEnabled() {
	_gameScreen->notifyDiaryEntryEnabled();
}

} // End of namespace Stark
