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

#ifndef STARK_SERVICES_GAME_CHAPTER_H
#define STARK_SERVICES_GAME_CHAPTER_H

#include "common/str.h"
#include "common/array.h"

namespace Stark {

/**
 * Game chapter services
 * 
 * Provide the game chapter's title and subtitle
 */
class GameChapter {
public:
	GameChapter();
	~GameChapter() {}

	Common::String getCurrentChapterTitle() {
		return _chapterEntries[getActualCurrentChapter()].title;
	}

	Common::String getCurrentChapterSubtitle() {
		return _chapterEntries[getActualCurrentChapter()].subtitle;
	}

private:
	struct ChapterEntry {
		Common::String title;
		Common::String subtitle;
	};

	Common::Array<ChapterEntry> _chapterEntries;

	int getActualCurrentChapter();
};

} // End of namespace Stark

#endif // STARK_SERVICES_GAME_CHAPTER_H
