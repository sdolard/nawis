/*
 * n_http.cpp - http info
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

#include "n_http.h"

const QString NHttp_n::STATUS_CODE[] = {
    "100", // Continue
    "101", // Switching Protocols
    "200", // OK
    "201", // Created
    "202", // Accepted
    "203", // Non-Authoritative Information
    "204", // No Content
    "205", // Reset Content
    "206", // Partial Content
    "300", // Multiple Choices
    "301", // Moved Permanently
    "302", // Found
    "303", // See Other
    "304", // Not Modified
    "305", // Use Proxy
    "307", // Temporary Redirect
    "400", // Bad Request
    "401", // Unauthorized
    "402", // Payment Required
    "403", // Forbidden
    "404", // Not Found
    "405", // Method Not Allowed
    "406", // Not Acceptable
    "407", // Proxy Authentication Required
    "408", // Request Time-out
    "409", // Conflict
    "410", // Gone
    "411", // Length Required
    "412", // Precondition Failed
    "413", // Request Entity Too Large
    "414", // Request-URI Too Large
    "415", // Unsupported Media Type
    "416", // Requested range not satisfiable
    "417", // Expectation Failed
    "500", // Internal Server Error
    "501", // Not Implemented
    "502", // Bad Gateway
    "503", // Service Unavailable
    "504", // Gateway Time-out
    "505", // HTTP Version not supported
    ""
};

const QString NHttp_n::statusCodeToString(int code)
{
    switch(code)
    {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Time-out";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Request Entity Too Large";
    case 414: return "Request-URI Too Large";
    case 415: return "Unsupported Media Type";
    case 416: return "Requested range not satisfiable";
    case 417: return "Expectation Failed";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Time-out";
    case 505: return "HTTP Version not supported";
    default: return "Not defined http status code";
    }
}
