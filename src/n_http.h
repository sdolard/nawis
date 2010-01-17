/*
 * n_http.h - http info
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
 
// Qt
#include <QString> 

/*
	From HTTP RFC http://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html#sec6
      - 1xx: Informational - Request received, continuing process

      - 2xx: Success - The action was successfully received,
        understood, and accepted

      - 3xx: Redirection - Further action must be taken in order to
        complete the request

      - 4xx: Client Error - The request contains bad syntax or cannot
        be fulfilled

      - 5xx: Server Error - The server failed to fulfill an apparently
        valid request
*/
#define N_HTTP_CONTINUE                             100
#define N_HTTP_SWITCHING_PROTOCOLS                  101
#define N_HTTP_OK                                   200
#define N_HTTP_CREATED                              201
#define N_HTTP_ACCEPTED                             202
#define N_HTTP_NON_AUTHORITATIVE_INFORMATION        203
#define N_HTTP_NO_CONTENT                           204
#define N_HTTP_RESET_CONTENT                        205
#define N_HTTP_PARTIAL_CONTENT                      206
#define N_HTTP_MULTIPLE_CHOICES                     300
#define N_HTTP_MOVED_PERMANENTLY                    301
#define N_HTTP_FOUND                                302
#define N_HTTP_SEE_OTHER                            303
#define N_HTTP_NOT_MODIFIED                         304
#define N_HTTP_USE_PROXY                            305
#define N_HTTP_TEMPORARY_REDIRECT                   307
#define N_HTTP_BAD_REQUEST                          400
#define N_HTTP_UNAUTHORIZED                         401 // Constant denoting HTTP Unauthorized status. The request requires user authentication.
#define N_HTTP_PAYMENT_REQUIRED                     402
#define N_HTTP_FORBIDDEN                            403
#define N_HTTP_NOT_FOUND                            404
#define N_HTTP_METHOD_NOT_ALLOWED                   405
#define N_HTTP_NOT_ACCEPTABLE                       406
#define N_HTTP_PROXY_AUTHENTICATION_REQUIRED        407
#define N_HTTP_REQUEST_TIME_OUT                     408
#define N_HTTP_CONFLICT                             409
#define N_HTTP_GONE                                 410
#define N_HTTP_LENGTH_REQUIRED                      411
#define N_HTTP_PRECONDITION_FAILED                  412
#define N_HTTP_REQUEST ENTITY_TOO_LARGE             413
#define N_HTTP_REQUEST_URI_TOO_LARGE                414
#define N_HTTP_UNSUPPORTED_MEDIA_TYPE               415
#define N_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE      416
#define N_HTTP_EXPECTATION_FAILED                   417
#define N_HTTP_INTERNAL_SERVER_ERROR                500
#define N_HTTP_NOT_IMPLEMENTED                      501
#define N_HTTP_BAD_GATEWAY                          502
#define N_HTTP_SERVICE_UNAVAILABLE                  503
#define N_HTTP_GATEWAY_TIME_OUT                     504
#define N_HTTP_HTTP_VERSION_NOT_SUPPORTED           505
	
namespace NHttp_n
{
	extern const QString STATUS_CODE[];
	const QString statusCodeToString(int code);
}

