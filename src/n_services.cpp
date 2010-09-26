/*
 *  n_services.cpp
 *  nawis
 *
 *  Created by Sébastien Dolard on 17/10/08.
 *  Copyright 2008. All rights reserved.
 *
 */

#include "n_log.h"

#include "n_services.h"
#include "n_tcp_server_auth_session.h"

/*
 * nsServiceNone
 */
NService_n::NService NService_n::nsServiceNone =
{
    SVC_NONE, // id
    "", // service
    false, // authRequired
    AUTH_LEVEL_NONE, // requiredLevel
    "0.1.0", // history
    "", // comment
    "", // fullService;
    "", // httpMethod
    "", // params
    "", // content
    "", // returns
    NULL
};

/* 
 * nsServices
 */
NService_n::NService NService_n::nsServices[] =
{
    {
        SVC_API, // id
        "api", // service
        false, // authRequired
        AUTH_LEVEL_NONE, // requiredLevel
        "0.1.0", // history
        "Display API commands", // comment
        "api", // fullService
        "GET", // httpMethod
        "", // params
        "", // content
        "API help", // returns
        NService_n::nsAPIServices
    },
{
        SVC_FAVICON, // id
        "favicon.ico", // service
        false, // authRequired
        AUTH_LEVEL_NONE, // requiredLevel
        "0.1.0", // history
        "Download application favicon", // comment
        "favicon.ico", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "favicon.ico file", // returns
        NULL
    },
{
        SVC_HELP, // id
        "help", // service
        false, // authRequired
        AUTH_LEVEL_NONE, // requiredLevel
        "0.1.0", // history
        "Display application API help", // comment
        "help", // fullService
        "GET", // httpMethod
        "", // params
        "", // content
        "Full api help", // returns
        NULL
    },
{
        SVC_UI, // id
        "ui", // service
        false, // authRequired
        AUTH_LEVEL_NONE, // requiredLevel
        "0.1.0", // history
        "Access to server default UI", // comment
        "ui", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "Ui files", // returns
        NULL
    },
NService_n::nsServiceNone
};


/* 
 * nsAPIServices
 */
NService_n::NService NService_n::nsAPIServices[] = {
    {
        SVC_API_AUTH, // id
        "auth", // service
        false, // authRequired
        AUTH_LEVEL_NONE, // requiredLevel
        "0.1.0", // history
        "Display authentication help", // comment
        "api/auth", // fullService
        "GET", // httpMethod
        "", // params
        "", // content
        "Authentication help", // returns
        NULL
    },{
        SVC_API_AUTH, // id
        "auth", // service
        false, // authRequired
        AUTH_LEVEL_NONE, // requiredLevel
        "0.1.0", // history
        "Use to auth a user with a login and a password", // comment
        "api/auth", // fullService
        "POST", // httpMethod
        "", // params
        "Support JSON and FORM<br>"\
        "JSON:<pre>{<br>"\
        "  \"username\":\"&lt;username&gt;\", // string <br>"\
        "  \"password\":\"&lt;password&gt;\" // string <br>"\
        "}</pre>"\
        "or FORM:<pre><br>"\
        "  username=&lt;username&gt;, // string <br>"\
        "  password=&lt;password&gt; // string</pre>", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;login message&gt;\", // string<br>"\
        "  \"level\":\"&lt;level&gt;\"// string: contains levels separated by space \" \" char<br>"\
        "}</pre>"\
        "In case of success, server will set \"nawis_sessionId\" cookie.<br>"\
        "Available levels are:<ul>"\
        "<li>user</li>"\
        "<li>admin</li></ul>", // returns
        NULL
    },{
        SVC_API_AUTH, // id
        "auth", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Use to unauth a user, based on \"nawis_sessionId\" cookie", // comment
        "api/auth", // fullService
        "DELETE", // httpMethod
        "", // params
        "", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;logout message&gt;\", // string<br>"
        "}</pre>"\
        "In case of success, server will clear value of \"nawis_sessionId\" cookie", // returns
        NULL
    },{
        SVC_API_CFG, // id
        "cfg", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Display server configuration API", // comment
        "api/cfg", // fullService
        "GET", // httpMethod
        "", // params
        "", // content
        "Configuration commands help", // returns
        NService_n::nsAPICfgServices
    },{
        SVC_API_DOWNLOAD, // id
        "download", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Download a resource", // comment
        "api/download/&lt;file hash&gt;", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "The file<br>"\
        "&lt;file hash&gt; is a md5 hash of file to download", // returns
        NULL
    },{
        SVC_API_DUPLICATED, // id
        "duplicated", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Duplicated api help", // comment
        "api/duplicated", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "", // returns
        nsAPIDuplicatedServices
    },{
        SVC_API_FILE, // id
        "file", // service
        false, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Root file api commands", // comment
        "api/file", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "File command help", // returns
        NService_n::nsAPIFileServices
    },{
        SVC_API_LOG, // id
        "log", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Return logs. Various thing a logged.", // comment
        "api/log", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;sorted field&gt; // optionnal, default \"date\", see returned item field for selection<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;load message&gt;\", // string<br>"\
        "  \"data\":[{<br>"\
        "    \"id\":&lt;id&gt;, // number: id is set by server<br>"\
        "    \"date\":\"&lt;log date time&gt;\", // format: \"yyyy-MM-dd hh:mm:ss.zzz\", string <br>"\
        "    \"log\":\"&lt;log message&gt;\" // string <br>"\
        "  },...<br>"
        "]}</pre>", // returns
        NULL
    },{
        SVC_API_LOG, // id
        "log", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Use to delete logs", // comment
        "api/log", // fullService
        "DELETE", // httpMethod
        "[help]", // params
        "", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;delete message&gt;\" // string <br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_MUSIC, // id
        "music", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Music commands.", // comment
        "api/music", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "Music command help", // returns
        NService_n::nsAPIMusicServices
    },{
        SVC_API_NOP, // id
        "nop", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "NOP", // comment
        "api/nop", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "JSON"
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;nop message&gt;\", // string <br>"\
        "  \"status\":\"&lt;SERVER_STATUS&gt;\" // string <br>"\
        "}</pre>"\
        "SERVER_STATUS are:<ul>"\
        "<li>JT_NONE: server is ready</li>"\
        "<li>JT_WATCH_FILES: server is looking for file system modification</li>"\
        "<li>JT_DB_UPDATE: server database update pending</li>"\
        "<li>JT_HASH: file hash pending</li>"\
        "<li>JT_GET_METADATA: file metadata extraction pending</li></ul>"\
        "<li>JT_BUILD_MUSIC_DB: music database build pending</li></ul>", // returns
        NULL
    },{
        SVC_API_PICTURE, // id
        "picture", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Picture commands", // comment
        "api/picture", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "Picture command help", // returns
        NService_n::nsAPIPictureServices
    },{
        SVC_API_SEARCH, // id
        "search", // service
        false, // authRequired
        AUTH_LEVEL_NONE, // requiredLevel
        "0.1.0", // history
        "Search root api", // comment
        "api/search", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "Search command help", // returns
        NService_n::nsAPISearchServices
    },{
        SVC_API_USER, // id
        "user", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Return user list.", // comment
        "api/user", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;sorted field&gt; // optionnal, default \"size\", see returned item field for selection<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;id of item&gt;, // number<br>"\
        "    \"email\":\"&lt;user email&gt;\", // string<br>"\
        "    \"name\":\"&lt;user name&gt;\",// string<br>"\
        "    \"preferences\":\"&lt;user preferences&gt;\", // string<br>"\
        "    \"disabled\":\"&lt;account state&gt;\", // boolean<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    }, {
        SVC_API_USER, // id
        "user", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Delete a user", // comment
        "api/user/&lt;id&gt", // fullService
        "DELETE", // httpMethod
        "[help]<br>"\
        "Just put id to delete (as last path) at the end of the url<br>"\
        "  Example: api/user/<b>/4</b>", // params
        "", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;delete message&gt;\" // string <br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_USER, // id
        "user", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Used to modify user properties", // comment
        "api/user/&lt;id&gt;", // fullService
        "PUT", // httpMethod
        "[help]<br>"\
        "Add id to update at the end of the url<br>"\
        "Example: api/user<b>/4</b>", // params
        "JSON"\
        "<pre>{<br>"\
        "  \"data\":{<br>"\
        "    \"email\":\"&lt;user email&gt;\", // string<br>"\
        "    \"name\":\"&lt;user name&gt;\",// string<br>"\
        "    \"preferences\":\"&lt;user preferences&gt;\", // string<br>"\
        "    \"disabled\":\"&lt;account state&gt;\", // boolean<br>"\
        "    \"id\":&lt;id to update&gt; // number <br>"\
        "  }<br>"\
        "}</pre>", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;put message&gt;\", // string<br>"\
        "  \"data\":[{ // array<br>"\
        "    \"id\":&lt;id&gt;, // number<br>"\
        "    \"email\":\"&lt;user email&gt;\", // string<br>"\
        "    \"name\":\"&lt;user name&gt;\",// string<br>"\
        "    \"preferences\":\"&l;tuser preferences&gt;\", // string<br>"\
        "    \"disabled\":\"&lt;account state&gt;\", // boolean<br>"\
        "  }]<br>"\
        "}</pre>", // returns
        NULL
    }, {
        SVC_API_USER, // id
        "user", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Add a user. Account is default deactivated.", // comment
        "api/user", // fullService
        "POST", // httpMethod
        "[help]", // params
        "JSON"\
        "<pre>{<br>"\
        "  \"data\": // an object with a data property<br>"\
        "  {<br>"\
        "    \"email\":\"&lt;user email&gt;\", // string<br>"\
        "    \"name\":\"&lt;user name&gt;\",// string<br>"\
        "    \"password\":\"&lt;user password&gt;\",// string<br>"\
        "  }<br>"\
        "}</pre>", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;post message&gt;\", // string<br>"\
        "  \"data\":[{<br>"\
        "    \"id\":&lt;id&gt;, // number: id is set by server<br>"\
        "    \"email\":\"&lt;user email&gt;\", // string<br>"\
        "    \"name\":\"&lt;user name&gt;\",// string<br>"\
        "    \"preferences\":\"&l;tuser preferences&gt;\", // string<br>"\
        "    \"disabled\":\"&lt;account state&gt;\", // boolean<br>"\
        "  }<br>"\
        "]}</pre>", // returns
        NULL
    },
NService_n::nsServiceNone
};

/* 
 *nsAPICfgServices
 */
NService_n::NService NService_n::nsAPICfgServices[] =
{
    {
        SVC_API_CFG_SHARED_DIR, // id
        "shareddir", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Return list of shared directories, with there sharing properties", // comment
        "api/cfg/shareddir", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;load message&gt;\", // string<br>"\
        "  \"totalcount\":&lt;number of element&gt;,// number, here equal to data array size<br>"\
        "  \"data\":[{ // array<br>"\
        "    \"id\":&lt;id&gt;, // number<br>"\
        "    \"path\":\"&lt;absolute directory path&gt;\", // string: absolute directory path<br>"\
        "    \"recursive\":true&#124;false, // boolean: true to shared contained directories<br>"\
        "    \"shared\":true&#124;false, // boolean: true if directory is shared<br>"\
        "    \"exists\":true&#124;false // boolean: true if directory exists on server<br>"\
        "  },<br>"\
        "  {... // others shared directory objects, if exists...<br>"\
        "  }]<br>"\
        "}</pre>", // returns
        NULL
    },
{
        SVC_API_CFG_SHARED_DIR, // id
        "shareddir", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Remove a directory from shared list", // comment
        "api/cfg/shareddir/&lt;id&gt;", // fullService
        "DELETE", // httpMethod
        "[help]<br>"\
        "Just put id to delete (as last path) at the end of the url<br>"\
        "Example: api/cfg/shareddir<b>/4</b>", // params
        "", // content
        "JSON"
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;delete message&gt;\" // string <br>"\
        "}</pre>", // returns
        NULL
    },
{
        SVC_API_CFG_SHARED_DIR, // id
        "shareddir", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Add a shared directory", // comment
        "api/cfg/shareddir", // fullService
        "POST", // httpMethod
        "[help]", // params
        "JSON"\
        "<pre>{<br>"\
        "  \"data\": // an object with a data property<br>"\
        "  {<br>"\
        "    \"path\":\"&lt;absolute directory path&gt;\", // string<br>"\
        "    \"shared\":true&#124;false, // boolean<br>"\
        "    \"recursive\":true&#124;false // boolean<br>"\
        "  }<br>"\
        "}</pre>", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;post message&gt;\", // string<br>"\
        "  \"data\":[{<br>"\
        "    \"id\":&lt;id&gt;, // number: id is set by server<br>"\
        "    \"path\":\"&lt;absolute directory path&gt;\", // string <br>"\
        "    \"recursive\":true&#124;false, // boolean<br>"\
        "    \"shared\":true&#124;false, // boolean<br>"\
        "    \"exists\":true&#124;false // boolean<br>"\
        "  }<br>"\
        "]}</pre>", // returns
        NULL
    },
{
        SVC_API_CFG_SHARED_DIR, // id
        "shareddir", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Used to modify shared directory properties", // comment
        "api/cfg/shareddir/&lt;id&gt;", // fullService
        "PUT", // httpMethod
        "[help]<br>"\
        "Add id to update at the end of the url<br>"\
        "Example: api/cfg/shareddir<b>/4</b>", // params
        "JSON"\
        "<pre>{<br>"\
        "  \"data\":{<br>"\
        "    \"path\":\"&lt;absolute directory path&gt;\", // optionnal, string<br>"\
        "    \"shared\":true&#124;false, // optionnal, boolean<br>"\
        "    \"recursive\":true&#124;false, // optionnal, boolean<br>"\
        "    \"id\":&lt;id to update&gt; // number <br>"\
        "  }<br>"\
        "}</pre>", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"message\":\"&lt;put message&gt;\", // string<br>"\
        "  \"data\":[{ // array<br>"\
        "    \"id\":&lt;id&gt;, // number<br>"\
        "    \"path\":\"&lt;absolute directory path&gt;\",  // string<br>"\
        "    \"recursive\":true&#124;false, // boolean<br>"\
        "    \"shared\":true&#124;false, // boolean<br>"\
        "    \"exists\":true&#124;false // boolean<br>"\
        "  }]<br>"\
        "}</pre>", // returns
        NULL
    },
NService_n::nsServiceNone
};


/*
 * nsAPIFileServices
 */
NService_n::NService NService_n::nsAPIFileServices[] =
{
    {
        SVC_API_FILE_UPDATE_DB, // id
        "updatedb", // service
        true, // authRequired
        AUTH_LEVEL_ADMIN, // requiredLevel
        "0.1.0", // history
        "Look for new files and modification then update database with collected<br>"\
        "data and metadata(id3, IPCT, exif...).<br>"\
        "First time this operation is done, it could take a long time.<br>"\
        "See <b>api/nop</b> to retreive pending server operation status.", // comment
        "api/file/updatedb", // fullService
        "GET", // httpMethod
        "[help]", // params
        "", // content
        "JSON"\
        "<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean <br>"\
        "  \"message\":\"&lt;update message&gt;\" // string<br>"\
        "}</pre>", // returns
        NULL
    },
NService_n::nsServiceNone
};

/*
 * nsAPIMusicServices
 */
NService_n::NService NService_n::nsAPIMusicServices[] =
{
    {
        SVC_API_MUSIC_ALBUM, // id
        "album", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Album list", // comment
        "api/music/album", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit<br>"\
        "year:  &lt;music year&gt; // optionnal<br>"\
        "genre: &lt;music genre&gt; // optionnal<br>"\
        "artist: &lt;music artist&gt; // optionnal", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":\"&lt;artist id&gt;\" // number<br>"\
        "    \"album\":\"&lt;album name&gt;\" // string<br>"\
        "    \"mainArtist\":\"&lt;main album artist&gt;\" // string, empty if no artist or more than one<br>"\
        "    \"frontCoverPictureFileHash\":\"&lt;front cover picture file hash&gt;\" // string, use with api/picture/thumb/<br>"\
        "    \"backCoverPictureFileHash\":\"&lt;back cover picture file hash&gt;\" // string, use with api/picture/thumb/<br>"\
        "    \"frontCoverID3PictureFileHash\":\"&lt;front cover id3 picture file hash&gt;\" // string, use with api/music/id3picture/<br>"\
        "    \"backCoverID3PictureFileHash\":\"&lt;back cover id3 picture file hash&gt;\" // string, use with api/music/id3picture/<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_MUSIC_ARTIST, // id
        "artist", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Album list", // comment
        "api/music/artist", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit<br>"\
        "year:  &lt;music year&gt; // optionnal<br>"\
        "genre: &lt;music genre&gt; // optionnal", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":\"&lt;artist id&gt;\" // number<br>"\
        "    \"artist\":\"&lt;artist name&gt;\" // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_MUSIC_GENRE, // id
        "genre", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Genre list", // comment
        "api/music/genre", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit<br>"\
        "year:  &lt;music year&gt; // optionnal", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":\"&lt;genre id&gt;\" // number<br>"\
        "    \"genre\":\"&lt;genre name&gt;\" // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_MUSIC_ID3_PICTURE, // id
        "id3picture", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return a thumbnail picture contained in id3 files", // comment
        "api/music/id3picture/&lt;file hash&gt;", // fullService
        "GET", // httpMethod
        "Put &lt;music file hash&gt; at the end of the url<br>"\
        "Example: api/music/id3picture<b>/62f6287d95fafa0a8c56287e41760f33</b>", // params
        "", // content
        "A picture. Format is in mimeType response.", // returns
        NULL
    },{
        SVC_API_MUSIC_TITLE, // id
        "title", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Title list", // comment
        "api/music/title", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;any data field&gt; // Field to sort, optionnal, default \"album\"<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit<br>"\
        "year:  &lt;music year&gt; // optionnal<br>"\
        "genre: &lt;music genre&gt; // optionnal<br>"\
        "artist: &lt;music artist&gt; // optionnal<br>"\
        "album: &lt;music album&gt; // optionnal", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;item id&gt;, // number<br>"\
        "    \"fileName\":\"&lt;music filename&gt;\", // string<br>"\
        "    \"hash\":\"&lt;music file md5 hash&gt;\", // string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number<br>"\
        "    \"artist\":\"&lt;artist name&gt;\", // string<br>"\
        "    \"comment\":\"&lt;music comment&gt;\", // string<br>"\
        "    \"year\":&lt;music year&gt;, // number<br>"\
        "    \"album\":\"&lt;album name&gt;\", // string<br>"\
        "    \"title\":\"&lt;music title&gt;\", // string<br>"\
        "    \"genre\":\"&lt;music genre&gt;\", // string<br>"\
        "    \"trackNumber\":&lt;trac number&gt;, // number<br>"\
        "    \"duration\":&lt;duration in second&gt;, // number<br>"\
        "    \"hasID3Picture\":true&#124;false // boolean, use with api/music/id3picture/<br>"\
        "    \"frontCoverPictureFileHash\":\"&lt;front cover picture file hash&gt;\" // string, use with api/picture/thumb/<br>"\
        "    \"frontCoverID3PictureFileHash\":\"&lt;front cover id3 picture file hash&gt;\" // string, use with api/music/id3picture/<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_MUSIC_YEAR, // id
        "year", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Year list", // comment
        "api/music/year", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit",  // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"year\":\"&lt;album year&gt;\" // (no id need, cos a year can not be empty (0 if no year is set), string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },
NService_n::nsServiceNone
};

/*
 * nsAPIPictureServices
 */
NService_n::NService NService_n::nsAPIPictureServices[] =
{
    {
        SVC_API_PICTURE_RESIZE, // id
        "resize", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return a resized jpeg picture", // comment
        "api/picture/resize/&lt;file hash&gt;", // fullService
        "GET", // httpMethod
        "Put &lt;picture file hash&gt; to resize at the end of the url<br>"\
        "Example: api/picture/resize<b>/62f6287d95fafa0a8c56287e41760f33</b><br>"
        "width: &lt;required picture width&gt; // optionnal, default \"800\"<br>"\
        "height: &lt;required picture height&gt; // optionnal, default \"600\"<br>"\
        "Resize is always proportional.", // params
        "", // content
        "A jpeg picture", // returns
        NULL
    },{
        SVC_API_PICTURE_THUMB, // id
        "thumb", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return a thumbnail jpeg picture", // comment
        "api/picture/thumb/&lt;file hash&gt;", // fullService
        "GET", // httpMethod
        "Put &lt;picture file hash&gt; at the end of the url<br>"\
        "Example: api/picture/thumb<b>/62f6287d95fafa0a8c56287e41760f33</b>", // params
        "", // content
        "A jpeg picture", // returns
        NULL
    },
NService_n::nsServiceNone
};


/*
 * nsAPISearchServices
 */
NService_n::NService NService_n::nsAPISearchServices[]=
{
    {
        SVC_API_SEARCH_ARCHIVE, // id
        "archive", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Search archives", // comment
        "api/search/archive", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;any data field&gt; // Field to sort, optionnal, default \"added\"<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;item id&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"hash\":\"&lt;file md5 hash&gt;\", // string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number<br>"\
        "    \"relativePath\":\"&lt;file relative path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;other&#124;movie&#124;music&#124;picture&#124;archive&#124;document&gt;\", // string, file category type<br>"\
        "    \"added\":\"&lt;Date time added in server database&gt;\", // string<br>"\
        "    \"lastModified\":\"&lt;last file edition&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_SEARCH_DOCUMENT, // id
        "document", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Search document", // comment
        "api/search/document", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;any data field&gt; // Field to sort, optionnal, default \"added\"<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;item id&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"hash\":\"&lt;file md5 hash&gt;\", // string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number<br>"\
        "    \"relativePath\":\"&lt;file relative path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;other&#124;movie&#124;music&#124;picture&#124;archive&#124;document&gt;\", // string, file category type<br>"\
        "    \"added\":\"&lt;Date time added in server database&gt;\", // string<br>"\
        "    \"lastModified\":\"&lt;last file edition&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_SEARCH_FILE, // id
        "file", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Search files", // comment
        "api/search/file", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;any data field&gt; // Field to sort, optionnal, default \"added\"<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;item id&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"hash\":\"&lt;file md5 hash&gt;\", // string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number<br>"\
        "    \"relativePath\":\"&lt;file relative path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;other&#124;movie&#124;music&#124;picture&#124;archive&#124;document&gt;\", // string, file category type<br>"\
        "    \"added\":\"&lt;Date time added in server database&gt;\", // string<br>"\
        "    \"lastModified\":\"&lt;last file edition&gt;\", // string<br>"\
        "    \"artist\":\"&lt;music artist name&gt;\", // string, only for music category file<br>"\
        "    \"comment\":\"&lt;music file comment&gt;\", // string, only for music category file<br>"\
        "    \"year\":&lt;music year&gt;, // number, only for music category file<br>"\
        "    \"album\":\"&lt;music album name&gt;\", // string, only for music category file<br>"\
        "    \"title\":\"&lt;music title&gt;\", // string, only for music category file<br>"\
        "    \"genre\":\"&lt;music genre&gt;\", // string, only for music category file<br>"\
        "    \"trackNumber\":&lt;music trac number&gt;, // number, only for music category file<br>"\
        "    \"duration\":&lt;music duration in second&gt;, // number, only for music category file<br>"\
        "    \"copyright\":\"&lt;music copyright&gt;\" // string, only for music category file<br>"\
        "    \"dateTimeOriginal\":\"&lt;picture date and time when the original image data was generated&gt;\", // string, only for picture category file<br>"\
        "    \"width\":&lt;picture width&gt;, // number, only for picture category file<br>"\
        "    \"height\":&lt;picture height&gt;, // number, only for picture category file<br>"\
        "    \"make\":\"&lt;picture manufacturer of the recording equipment&gt;\", // string, only for picture category file<br>"\
        "    \"model\":\"&lt;picture model name or model number of the equipment&gt;\", // string, only for picture category file<br>"\
        "    \"longitude\":\"&lt;picture longiture&gt;\", // string, only for picture category file<br>"\
        "    \"latitude\":\"&lt;picture latitude&gt;\", // string, only for picture category file<br>"\
        "    \"altitude\":&lt;picture altitude&gt;, // number, only for picture category file<br>"\
        "    \"city\":\"&lt;picture city&gt;\", // string, only for picture category file<br>"\
        "    \"provinceState\":\"&lt;picture state&gt;\", // string, only for picture category file<br>"\
        "    \"coutry\":\"&lt;picture country&gt;\", // string, only for picture category file<br>"\
        "    \"hasID3Picture\":true&#124;false // boolean<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_SEARCH_MOVIE, // id
        "movie", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Search movie", // comment
        "api/search/movie", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;any data field&gt; // Field to sort, optionnal, default \"added\"<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;item id&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"hash\":\"&lt;file md5 hash&gt;\", // string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number<br>"\
        "    \"relativePath\":\"&lt;file relative path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;other&#124;movie&#124;music&#124;picture&#124;archive&#124;document&gt;\", // string, file category type<br>"\
        "    \"added\":\"&lt;Date time added in server database&gt;\", // string<br>"\
        "    \"lastModified\":\"&lt;last file edition&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_SEARCH_MUSIC, // id
        "music", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Search music", // comment
        "api/search/music", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;any data field&gt; // Field to sort, optionnal, default \"added\"<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;item id&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"hash\":\"&lt;file md5 hash&gt;\", // string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number<br>"\
        "    \"relativePath\":\"&lt;file relative path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;other&#124;movie&#124;music&#124;picture&#124;archive&#124;document&gt;\", // string, file category type<br>"\
        "    \"added\":\"&lt;Date time added in server database&gt;\", // string<br>"\
        "    \"lastModified\":\"&lt;last file edition&gt;\", // string<br>"\
        "    \"artist\":\"&lt;music artist name&gt;\", // string<br>"\
        "    \"comment\":\"&lt;music file comment&gt;\", // string<br>"\
        "    \"year\":&lt;music year&gt;, // number<br>"\
        "    \"album\":\"&lt;music album name&gt;\", // string<br>"\
        "    \"title\":\"&lt;music title&gt;\", // string<br>"\
        "    \"genre\":\"&lt;music genre&gt;\", // string<br>"\
        "    \"trackNumber\":&lt;music trac number&gt;, // number<br>"\
        "    \"duration\":&lt;music duration in second&gt;, // number<br>"\
        "    \"copyright\":\"&lt;music copyright&gt;\" // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_SEARCH_OTHER, // id
        "other", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Search other format (see server config file)", // comment
        "api/search/other", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;any data field&gt; // Field to sort, optionnal, default \"added\"<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;item id&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"hash\":\"&lt;file md5 hash&gt;\", // string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number<br>"\
        "    \"relativePath\":\"&lt;file relative path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;other&#124;movie&#124;music&#124;picture&#124;archive&#124;document&gt;\", // string, file category type<br>"\
        "    \"added\":\"&lt;Date time added in server database&gt;\", // string<br>"\
        "    \"lastModified\":\"&lt;last file edition&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_SEARCH_PICTURE, // id
        "picture", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Search picture", // comment
        "api/search/picture", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;any data field&gt; // Field to sort, optionnal, default \"added\"<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;item id&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"hash\":\"&lt;file md5 hash&gt;\", // string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number<br>"\
        "    \"relativePath\":\"&lt;file relative path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;other&#124;movie&#124;music&#124;picture&#124;archive&#124;document&gt;\", // string, file category type<br>"\
        "    \"added\":\"&lt;Date time added in server database&gt;\", // string<br>"\
        "    \"lastModified\":\"&lt;last file edition&gt;\", // string<br>"\
        "    \"dateTimeOriginal\":\"&lt;picture date and time when the original image data was generated&gt;\", // string<br>"\
        "    \"width\":&lt;picture width&gt;, // number<br>"\
        "    \"height\":&lt;picture height&gt;, // number<br>"\
        "    \"make\":\"&lt;picture manufacturer of the recording equipment&gt;\", // string<br>"\
        "    \"model\":\"&lt;picture model name or model number of the equipment&gt;\", // string<br>"\
        "    \"longitude\":\"&lt;picture longiture&gt;\", // string<br>"\
        "    \"latitude\":\"&lt;picture latitude&gt;\", // string<br>"\
        "    \"altitude\":&lt;picture altitude&gt;, // number<br>"\
        "    \"city\":\"&lt;picture city&gt;\", // string<br>"\
        "    \"provinceState\":\"&lt;picture state&gt;\", // string<br>"\
        "    \"coutry\":\"&lt;picture country&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },
NService_n::nsServiceNone
};


/*
* nsAPIDuplicatedServices
*/
NService_n::NService NService_n::nsAPIDuplicatedServices[]=
{
    {
        SVC_API_DUPLICATED_ARCHIVE, // id
        "archive", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return duplicated archives", // comment
        "api/duplicated/archive", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;sorted field&gt; // optionnal, default \"size\", see returned item field for selection<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;id of item&gt;, // number, fileId and originialFileId concat<br>"\
        "    \"fileId\":&lt;file id&gt;, // number<br>"\
        "    \"originialFileId\":&lt;original file id&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"relativePath\":\"&lt;relative path&gt;\",// relative in relation to shared dir, string<br>"\
        "    \"absoluteFilePath\":\"&lt;absolute file path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;file category&gt;\", // string, TODO: explain<br>"\
        "    \"hash\":\"&lt;md5 file hash&gt;\",<br>"\
        "    \"added\":\"&lt;added in db date time&gt\", //format: \"yyyy-MM-dd hh:mm:ss.zzz\", string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number <br>"\
        "    \"originalAbsoluteFilePath\":\"&lt;absolute file path of original file&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_DUPLICATED_DOCUMENT, // id
        "document", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return duplicated document", // comment
        "api/duplicated/document", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;sorted field&gt; // optionnal, default \"size\", see returned item field for selection<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;id of item&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"relativePath\":\"&lt;relative path&gt;\",// relative in relation to shared dir, string<br>"\
        "    \"absoluteFilePath\":\"&lt;absolute file path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;file category&gt;\", // string, TODO: explain<br>"\
        "    \"hash\":\"&lt;md5 file hash&gt;\",<br>"\
        "    \"added\":\"&lt;added in db date time&gt\", //format: \"yyyy-MM-dd hh:mm:ss.zzz\", string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number <br>"\
        "    \"originalAbsoluteFilePath\":\"&lt;absolute file path of original file&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_DUPLICATED_FILE, // id
        "file", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return duplicated files", // comment
        "api/duplicated/file", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;sorted field&gt; // optionnal, default \"size\", see returned item field for selection<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;id of item&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"relativePath\":\"&lt;relative path&gt;\",// relative in relation to shared dir, string<br>"\
        "    \"absoluteFilePath\":\"&lt;absolute file path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;file category&gt;\", // string, TODO: explain<br>"\
        "    \"hash\":\"&lt;md5 file hash&gt;\",<br>"\
        "    \"added\":\"&lt;added in db date time&gt\", //format: \"yyyy-MM-dd hh:mm:ss.zzz\", string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number <br>"\
        "    \"originalAbsoluteFilePath\":\"&lt;absolute file path of original file&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_DUPLICATED_MOVIE, // id
        "movie", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return duplicated movie", // comment
        "api/duplicated/movie", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;sorted field&gt; // optionnal, default \"size\", see returned item field for selection<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;id of item&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"relativePath\":\"&lt;relative path&gt;\",// relative in relation to shared dir, string<br>"\
        "    \"absoluteFilePath\":\"&lt;absolute file path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;file category&gt;\", // string, TODO: explain<br>"\
        "    \"hash\":\"&lt;md5 file hash&gt;\",<br>"\
        "    \"added\":\"&lt;added in db date time&gt\", //format: \"yyyy-MM-dd hh:mm:ss.zzz\", string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number <br>"\
        "    \"originalAbsoluteFilePath\":\"&lt;absolute file path of original file&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_DUPLICATED_MUSIC, // id
        "music", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return duplicated music", // comment
        "api/duplicated/music", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;sorted field&gt; // optionnal, default \"size\", see returned item field for selection<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;id of item&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"relativePath\":\"&lt;relative path&gt;\",// relative in relation to shared dir, string<br>"\
        "    \"absoluteFilePath\":\"&lt;absolute file path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;file category&gt;\", // string, TODO: explain<br>"\
        "    \"hash\":\"&lt;md5 file hash&gt;\",<br>"\
        "    \"added\":\"&lt;added in db date time&gt\", //format: \"yyyy-MM-dd hh:mm:ss.zzz\", string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number <br>"\
        "    \"originalAbsoluteFilePath\":\"&lt;absolute file path of original file&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_DUPLICATED_OTHER, // id
        "other", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return duplicated other format (see server config file)", // comment
        "api/duplicated/other", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;sorted field&gt; // optionnal, default \"size\", see returned item field for selection<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;id of item&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"relativePath\":\"&lt;relative path&gt;\",// relative in relation to shared dir, string<br>"\
        "    \"absoluteFilePath\":\"&lt;absolute file path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;file category&gt;\", // string, TODO: explain<br>"\
        "    \"hash\":\"&lt;md5 file hash&gt;\",<br>"\
        "    \"added\":\"&lt;added in db date time&gt\", //format: \"yyyy-MM-dd hh:mm:ss.zzz\", string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number <br>"\
        "    \"originalAbsoluteFilePath\":\"&lt;absolute file path of original file&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },{
        SVC_API_DUPLICATED_PICTURE, // id
        "picture", // service
        true, // authRequired
        AUTH_LEVEL_USER, // requiredLevel
        "0.1.0", // history
        "Return duplicated picture", // comment
        "api/duplicated/picture", // fullService
        "GET", // httpMethod
        "search: &lt;text to search&gt; // optionnal<br>"\
        "sort: &lt;sorted field&gt; // optionnal, default \"size\", see returned item field for selection<br>"\
        "dir: &lt;ASC&#124;DESC&gt; // sort direction, optionnal, default \"ASC\"<br>"\
        "start: &lt;start offset&gt; // optionnal, default \"0\"<br>"\
        "limit: &lt;result limit&gt; // optionnal, default \"25\", -1 for no limit", // params
        "", // content
        "JSON<pre>{<br>"\
        "  \"success\":true&#124;false, // boolean<br>"\
        "  \"totalcount\":&lt;Real total number of available items.&gt;, // number<br>"\
        "              // This value is not in relation to limit params.<br>"\
        "              // data.length is the number of items in relation to search and limit.<br>"\
        "  \"message\":\"&lt;loaded message&gt;\",<br>"\
        "  \"data\":[{ // array of result<br>"\
        "    \"id\":&lt;id of item&gt;, // number<br>"\
        "    \"fileName\":\"&lt;file name&gt;\", // string<br>"\
        "    \"relativePath\":\"&lt;relative path&gt;\",// relative in relation to shared dir, string<br>"\
        "    \"absoluteFilePath\":\"&lt;absolute file path&gt;\", // string<br>"\
        "    \"fileCategory\":\"&lt;file category&gt;\", // string, TODO: explain<br>"\
        "    \"hash\":\"&lt;md5 file hash&gt;\",<br>"\
        "    \"added\":\"&lt;added in db date time&gt\", //format: \"yyyy-MM-dd hh:mm:ss.zzz\", string<br>"\
        "    \"size\":&lt;file size in octet&gt;, // string, should be a number <br>"\
        "    \"originalAbsoluteFilePath\":\"&lt;absolute file path of original file&gt;\", // string<br>"\
        "  },<br>"\
        "  ... // other items<br>"\
        "  ]<br>"\
        "}</pre>", // returns
        NULL
    },
NService_n::nsServiceNone
};


/*
 * toService
 */
NService_n::NService NService_n::getHTTPMethodService(NService* rootServices, const QStringList & paths,
                                                      const QString & httpMethod, int level)
{
    if (paths.count() == 0)
        return NService_n::nsServiceNone;

    int i = 0;
    while (true)
    {
        if (rootServices[i].id == SVC_NONE)
            return NService_n::nsServiceNone;

        if (rootServices[i].service.compare(paths[level]) == 0)
        {
            if (rootServices[i].subServices == NULL)
            {
                if (rootServices[i].httpMethod.compare(httpMethod) == 0)
                    return rootServices[i];
            } else {
                if (level+1 >= paths.count())
                    return rootServices[i];

                return NService_n::getHTTPMethodService(rootServices[i].subServices, paths, httpMethod, level + 1);
            }
        }
        ++i;
    }
    return NService_n::nsServiceNone;
}

void NService_n::getServices(NService* rootServices, const QStringList & paths,
                             NService* services, int * size, int level)
{
    if (paths.count() == 0)
        return;

    int i = 0;
    while (true)
    {
        if (rootServices[i].id == SVC_NONE)
            return;

        if (rootServices[i].service.compare(paths[level]) == 0)
        {
            if (rootServices[i].subServices == NULL  ||
                level+1 >= paths.count())
            {
                services[*size] = rootServices[i];
                (*size)++;
                services[*size] = NService_n::nsServiceNone;
            } else if (rootServices[i].subServices != NULL)
                NService_n::getServices(rootServices[i].subServices, paths, services, size, level + 1);
        }
        ++i;
    }
}

void NService_n::getAllServices(NService* rootServices, NService* services, int * size)
{
    int i = 0;
    while (true)
    {
        if (rootServices[i].id == SVC_NONE)
            return;
        services[*size] = rootServices[i];
        (*size)++;
        services[*size] = NService_n::nsServiceNone;
        if (rootServices[i].subServices != NULL)
            NService_n::getAllServices(rootServices[i].subServices, services, size);
        ++i;
    }
}
