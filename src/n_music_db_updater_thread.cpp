#include <QFileInfo>

#include "n_log.h"
#include "n_music_database.h"

#include "n_music_db_updater_thread.h"

NMusicDbUpdaterThread::NMusicDbUpdaterThread(QObject * parent)
    :NThread(parent)
{
}

NMusicDbUpdaterThread::~NMusicDbUpdaterThread()
{
}

void NMusicDbUpdaterThread::run()
{
    NLOGM("NServer", tr("Music db building..."));
    NMDB.updateDb();
    NLOGM("NServer", tr("Music db build done."));
}
