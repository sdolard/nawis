 message(nawis need Qt 4.4.* or more: your Qt version is $$[QT_VERSION])

TEMPLATE = app
# for 64 bits build, replace x86 with x86_64
CONFIG += console qt thread

CONFIG-=app_bundle
QT += network sql script

# Application debug/release options 
CONFIG(debug, debug|release) {
	TARGET = nawis_debug
	CONFIG += warn_on
	message(Debug build)
	DEFINES += DEBUG
}
CONFIG(release, debug|release) {
	TARGET = nawis
	CONFIG += warn_off
        #DEFINES += QT_NO_DEBUG_OUTPUT #Comment until http://bugreports.qt.nokia.com/browse/QTSOLBUG-72 is fixed
	message(Release build)
}

# Output directory
# DESTDIR = out # ? not interesting to build there...
message(The binary will be compile in $$DESTDIR/)

# Temp directory
MOC_DIR = tmp
OBJECTS_DIR = tmp      

# Resources
RESOURCES = nawis.qrc

# Sources
HEADERS += src/n_client_session.h \
    src/n_compat.h \
    src/n_compress.h \
    src/n_config.h \
    src/n_convert.h \
    src/n_database.h \
    src/n_date.h \
    src/n_db_updater.h \
    src/n_dir.h \
    src/n_dir_watcher.h \
    src/n_file_category.h \
    src/n_file_hash.h \
    src/n_file_suffix.h \
    src/n_hasher_thread.h \
    src/n_http.h \
    src/n_image.h \
    src/n_json.h \
    src/n_log.h \
    src/n_log_database.h \
    src/n_metadata.h \
    src/n_metadata_updater_thread.h \
    src/n_mime_type.h \
    src/n_music_database.h \
    src/n_music_db_updater_thread.h \
    src/n_nawis_daemon.h \
    src/n_path.h \
    src/n_response.h \
    src/n_settings.h \
    src/n_server.h \
    src/n_services.h \
    src/n_string.h \
    src/n_string_map.h \
    src/n_tcp_server.h \
    src/n_tcp_server_auth_session.h \
    src/n_tcp_server_socket.h \
    src/n_tcp_server_socket_list.h \
    src/n_tcp_server_socket_services.h \
    src/n_thread.h \
    src/n_version.h \
    src/n_sqlite_error.h \
    src/n_tcp_server_socket_auth_services.h \
    src/n_tcp_server_socket_service.h \
    src/n_tcp_server_socket_log_services.h \
    src/n_tcp_server_socket_user_services.h \
    src/n_tcp_server_socket_music_services.h \
    src/n_tcp_server_socket_picture_services.h \
    src/n_tcp_server_socket_file_services.h \
    src/n_tcp_server_socket_ui_services.h \
    src/n_tcp_server_socket_cfg_services.h

SOURCES += src/main.cpp \
    src/n_client_session.cpp \
    src/n_compress.cpp \
    src/n_config.cpp \
    src/n_convert.cpp \
    src/n_database.cpp \
    src/n_date.cpp \
    src/n_db_updater.cpp \
    src/n_dir.cpp \
    src/n_dir_watcher.cpp \
    src/n_file_category.cpp \
    src/n_file_hash.cpp \
    src/n_file_suffix.cpp \
    src/n_hasher_thread.cpp \
    src/n_http.cpp \
    src/n_image.cpp \
    src/n_json.cpp \
    src/n_log.cpp \
    src/n_log_database.cpp \
    src/n_metadata.cpp \
    src/n_metadata_updater_thread.cpp \
    src/n_mime_type.cpp \
    src/n_music_database.cpp \
    src/n_music_db_updater_thread.cpp \
    src/n_nawis_daemon.cpp \
    src/n_path.cpp \
    src/n_response.cpp \
    src/n_settings.cpp \
    src/n_server.cpp \
    src/n_services.cpp \
    src/n_string.cpp \
    src/n_tcp_server.cpp \
    src/n_tcp_server_auth_session.cpp \
    src/n_tcp_server_socket.cpp \
    src/n_tcp_server_socket_services.cpp \
    src/n_thread.cpp \
    src/n_version.cpp \
    src/n_tcp_server_socket_auth_services.cpp \
    src/n_tcp_server_socket_service.cpp \
    src/n_tcp_server_socket_log_services.cpp \
    src/n_tcp_server_socket_user_services.cpp \
    src/n_tcp_server_socket_music_services.cpp \
    src/n_tcp_server_socket_picture_services.cpp \
    src/n_tcp_server_socket_file_services.cpp \
    src/n_tcp_server_socket_ui_services.cpp \
    src/n_tcp_server_socket_cfg_services.cpp


# Contrib
macx{
	INCLUDEPATH += /user/local/include/
}

# zlib sources
include(zlib.pri)

# exiv2 sources
include(exiv2.pri)

# taglib sources
include(taglib.pri)

# qtservice sources
include(contrib/qtservice-2.6_1-opensource/src/qtservice.pri)

# qtsingleapplication sources
include(contrib/qtsingleapplication-2.6_1-opensource/src/qtsinglecoreapplication.pri)
