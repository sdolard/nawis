#include <QFileInfo>

#include "n_log.h"
#include "n_music_database.h"

#include "n_album_cover_updater_thread.h"

NAlbumCoverUpdaterThread::NAlbumCoverUpdaterThread(QObject * parent)
    :NThread(parent)
{
}

NAlbumCoverUpdaterThread::~NAlbumCoverUpdaterThread()
{
}

void NAlbumCoverUpdaterThread::run()
{
    NMDB.updateDb();
}
