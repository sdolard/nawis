#ifndef N_MUSIC_DB_UPDATER_THREAD_H
#define N_MUSIC_DB_UPDATER_THREAD_H

/*
 * n_music_db_updater_thread.h
 * nawis
 *
 * Created by Sébastien Dolard on 04/09/09.
 * Copyright (C) 2008 Sebastien Dolard
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
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//Qt
#include <QString>

// App
#include "n_thread.h"

class NMusicDbUpdaterThread: public NThread {
    Q_OBJECT
public:
    NMusicDbUpdaterThread(QObject * parent = 0 );
    ~NMusicDbUpdaterThread();

protected:
    void run();
};

#endif // N_MUSIC_DB_UPDATER_THREAD_H
