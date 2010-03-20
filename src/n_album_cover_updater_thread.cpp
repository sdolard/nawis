#include <QFileInfo>

#include "n_log.h"
#include "n_database.h"

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
    NLOGM("NServer", tr("Album cover extraction..."));

    // Extract album list and add it in music_album
    // for each album look for a cover
    // manage album if already exists in db
    // manage album if no more exists through title or any other way

    NDB.updateMusicAlbumTable();
    NLOGM("NServer", tr("Album cover extraction done."));
}
