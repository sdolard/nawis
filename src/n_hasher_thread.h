/*
 * n_dir_analyzer.h - file sharing controler
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

#ifndef N_HASHER_THREAD_H
#define N_HASHER_THREAD_H

// App
#include "n_thread.h"

class NHasherThread: public NThread
{
	Q_OBJECT
public:
	NHasherThread( QObject * parent = 0 );

protected:
	void run();

private:
	bool hashFile();
	
signals:
	void hashingDone();
};

#endif // N_HASHER_THREAD_H
