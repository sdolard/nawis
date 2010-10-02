#ifndef N_TCP_SERVER_SOCKET_MUSIC_SERVICES_H
#define N_TCP_SERVER_SOCKET_MUSIC_SERVICES_H


#include "n_tcp_server_socket_service.h"

class NTcpServerSocketMusicServices : public NTcpServerSocketService
{
public:
    static NTcpServerSocketMusicServices & instance();
    static void deleteInstance();

    ~NTcpServerSocketMusicServices();

    NResponse & getAlbum(const NClientSession & session, NResponse & response);
    NResponse & getArtist(const NClientSession & session, NResponse & response);
    NResponse & getGenre(const NClientSession & session, NResponse & response);
    NResponse & getTitle(const NClientSession & session, NResponse & response);
    NResponse & getYear( const NClientSession & session, NResponse & response);
    NResponse & getID3Picture(int *statusCode, const NClientSession & session,
                                      NResponse & response);

private:
    static NTcpServerSocketMusicServices *m_instance;
    NTcpServerSocketMusicServices();

};

inline NTcpServerSocketMusicServices & getMusicServices(){
    return NTcpServerSocketMusicServices::instance();
}

#endif // N_TCP_SERVER_SOCKET_MUSIC_SERVICES_H
