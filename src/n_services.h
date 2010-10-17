/*
 *  n_services.h
 *  nawis
 *
 *  Created by Sébastien Dolard on 17/10/08.
 *  Copyright 2008. All rights reserved.
 *
 */
#ifndef N_SERVICES_H
#define N_SERVICES_H

// Qt
#include <QStringList>
#include <QString>

// SVC
#define SVC_NONE                                                    (0x00000000)
#define SVC_API                                                     (0x00000001)
#define SVC_FAVICON                                                 (0x00000002)
#define SVC_HELP                                                    (0x00000003)
#define SVC_UI                                                      (0x00000004)

// SVC_API
#define   SVC_API_AUTH                                    (SVC_API | 0x00000010)
#define   SVC_API_CFG                                     (SVC_API | 0x00000020)
#define   SVC_API_DOWNLOAD                                (SVC_API | 0x00000030)
#define   SVC_API_DUPLICATED                              (SVC_API | 0x00000040)
#define   SVC_API_FILE                                    (SVC_API | 0x00000050)
#define   SVC_API_LOG                                     (SVC_API | 0x00000060)
#define   SVC_API_MUSIC                                   (SVC_API | 0x00000070)
#define   SVC_API_NOP                                     (SVC_API | 0x00000080)
#define   SVC_API_PICTURE                                 (SVC_API | 0x00000090)
#define   SVC_API_SEARCH                                  (SVC_API | 0x000000A0)
#define   SVC_API_USER                                    (SVC_API | 0x000000B0)

// SVC_API_CFG
#define     SVC_API_CFG_SHARED_DIR                    (SVC_API_CFG | 0x00001000)

// SVC_API_DUPLICATED
#define     SVC_API_DUPLICATED_ARCHIVE         (SVC_API_DUPLICATED | 0x00001000)
#define     SVC_API_DUPLICATED_DOCUMENT        (SVC_API_DUPLICATED | 0x00002000)
#define     SVC_API_DUPLICATED_FILE            (SVC_API_DUPLICATED | 0x00003000)
#define     SVC_API_DUPLICATED_MOVIE           (SVC_API_DUPLICATED | 0x00004000)
#define     SVC_API_DUPLICATED_MUSIC           (SVC_API_DUPLICATED | 0x00005000)
#define     SVC_API_DUPLICATED_OTHER           (SVC_API_DUPLICATED | 0x00006000)
#define     SVC_API_DUPLICATED_PICTURE         (SVC_API_DUPLICATED | 0x00007000)

// SVC_API_FILE
#define     SVC_API_FILE_UPDATE_DB                   (SVC_API_FILE | 0x00001000)

// SVC_API_MUSIC
#define     SVC_API_MUSIC_ALBUM                     (SVC_API_MUSIC | 0x00001000)
#define     SVC_API_MUSIC_ARTIST                    (SVC_API_MUSIC | 0x00002000)
#define     SVC_API_MUSIC_GENRE                     (SVC_API_MUSIC | 0x00003000)
#define     SVC_API_MUSIC_ID3_PICTURE               (SVC_API_MUSIC | 0x00004000)
//#define     SVC_API_MUSIC_LYRIC                     (SVC_API_MUSIC | 0x00005000) // TODO
#define     SVC_API_MUSIC_TITLE                     (SVC_API_MUSIC | 0x00006000)
#define     SVC_API_MUSIC_YEAR                      (SVC_API_MUSIC | 0x00007000)

// SVC_API_PICTURE
#define     SVC_API_PICTURE_RESIZE                (SVC_API_PICTURE | 0x00001000)
#define     SVC_API_PICTURE_THUMB                 (SVC_API_PICTURE | 0x00002000)


// SVC_API_SEARCH
#define     SVC_API_SEARCH_ARCHIVE                 (SVC_API_SEARCH | 0x00001000)
#define     SVC_API_SEARCH_DOCUMENT                (SVC_API_SEARCH | 0x00002000)
#define     SVC_API_SEARCH_FILE                    (SVC_API_SEARCH | 0x00003000)
#define     SVC_API_SEARCH_MOVIE                   (SVC_API_SEARCH | 0x00004000)
#define     SVC_API_SEARCH_MUSIC                   (SVC_API_SEARCH | 0x00005000)
#define     SVC_API_SEARCH_OTHER                   (SVC_API_SEARCH | 0x00006000)
#define     SVC_API_SEARCH_PICTURE                 (SVC_API_SEARCH | 0x00007000)

namespace NService_n
{
    struct NService
    {
        int      id;
        QString  service;
        bool     authRequired;
        int      requiredLevel;
        QString  history;
        QString  comment;
        QString  fullService;
        QString  httpMethod;
        QString  params;
        QString  content;
        QString  returns;
        NService* subServices;
    };

    extern NService nsServiceNone;
    extern NService nsServices[];
    extern NService nsAPIServices[];
    extern NService nsAPICfgServices[];
    extern NService nsAPIFileServices[];
    extern NService nsAPIMusicServices[];
    extern NService nsAPIPictureServices[];
    extern NService nsAPISearchServices[];
    extern NService nsAPIDuplicatedServices[];

    // Return defined service for a path and one http method
    NService getHTTPMethodService(NService* rootServices, const QStringList & paths,
                                  const QString & httpMethod, int level = 0);

    // Set all services for a path in "services" array variable
    // Size will set to  size of services array
    void getServices(NService* rootServices, const QStringList & paths,
                     NService* services, int * size, int level = 0);

    // Set all services in "services" array variable
    // Size will set to  size of services array
    void getAllServices(NService* rootServices, NService* services, int * size);
};


#endif // N_SERVICES_H
