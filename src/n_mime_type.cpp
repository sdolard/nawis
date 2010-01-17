
//Qt 
#include <QHash>

// App
#include "n_log.h"

#include "n_mime_type.h"

// TODO: use mime type to manage search

QHash<QString, bool> HASH_MIME_COMPRESSED;
QHash<QString, QString> HASH_MIME_CONTENT_TYPE;
bool HashMimeContentTypeInitialized = false;

void initHashMimeContentType(){
	if (HashMimeContentTypeInitialized)
		return;
	
	HASH_MIME_CONTENT_TYPE["stl"] = "application/SLA";
	HASH_MIME_CONTENT_TYPE["css"] = "text/css";
	HASH_MIME_COMPRESSED["text/css"] = false;
	HASH_MIME_CONTENT_TYPE["htm"] = "text/html";
	HASH_MIME_COMPRESSED["text/html"] = false;
	HASH_MIME_CONTENT_TYPE["html"] = "text/html";
	HASH_MIME_COMPRESSED["text/html"] = false;
	HASH_MIME_CONTENT_TYPE["gif"] = "image/gif";
	HASH_MIME_CONTENT_TYPE["ief"] = "image/ief";
	HASH_MIME_CONTENT_TYPE["jpe"] = "image/jpeg";
	HASH_MIME_CONTENT_TYPE["jpeg"] = "image/jpeg";
	HASH_MIME_CONTENT_TYPE["jpg"] = "image/jpeg";
	HASH_MIME_CONTENT_TYPE["png"] = "image/png";
	HASH_MIME_CONTENT_TYPE["step"] = "application/STEP";
	HASH_MIME_CONTENT_TYPE["stp"] = "application/STEP";
	HASH_MIME_CONTENT_TYPE["dwg"] = "application/acad";
	HASH_MIME_CONTENT_TYPE["ez"] = "application/andrew-inset";	
	HASH_MIME_CONTENT_TYPE["js"] = "application/javascript";
	HASH_MIME_COMPRESSED["application/javascript"] = false;
	HASH_MIME_CONTENT_TYPE["swf"] = "application/x-shockwave-flash";
	HASH_MIME_CONTENT_TYPE["mp3"] = "audio/mpeg";
	HASH_MIME_CONTENT_TYPE["m3u"] = "audio/x-mpegurl";
	HASH_MIME_CONTENT_TYPE["ogg"] = "application/ogg";
	HASH_MIME_CONTENT_TYPE["mp2"] = "video/mpeg";
	HASH_MIME_CONTENT_TYPE["mp4"] = "video/mp4";
	HASH_MIME_CONTENT_TYPE["mpe"] = "video/mpeg";
	HASH_MIME_CONTENT_TYPE["mpeg"] = "video/mpeg";
	HASH_MIME_CONTENT_TYPE["mpg"] = "video/mpeg";
	HASH_MIME_CONTENT_TYPE["mov"] = "video/quicktime";
	HASH_MIME_CONTENT_TYPE["qt"] = "video/quicktime";
	HASH_MIME_CONTENT_TYPE["viv"] = "video/vnd.vivo";
	HASH_MIME_CONTENT_TYPE["vivo"] = "video/vnd.vivo";
	HASH_MIME_CONTENT_TYPE["fli"] = "video/x-fli";
	HASH_MIME_CONTENT_TYPE["asf"] = "video/x-ms-asf";
	HASH_MIME_CONTENT_TYPE["asx"] = "video/x-ms-asx";
	HASH_MIME_CONTENT_TYPE["wmv"] = "video/x-ms-wmv";
	HASH_MIME_CONTENT_TYPE["wmx"] = "video/x-ms-wmx";
	HASH_MIME_CONTENT_TYPE["wvx"] = "video/x-ms-wvx	";
	HASH_MIME_CONTENT_TYPE["avi"] = "video/x-msvideo";
	HASH_MIME_CONTENT_TYPE["flv"] = "video/flv";
	HASH_MIME_CONTENT_TYPE["divx"] = "video/divx";
	HASH_MIME_CONTENT_TYPE["ccad"] = "application/clariscad";
	HASH_MIME_CONTENT_TYPE["drw"] = "application/drafting";
	HASH_MIME_CONTENT_TYPE["tsp"] = "application/dsptype";
	HASH_MIME_CONTENT_TYPE["dxf"] = "application/dxf";
	HASH_MIME_CONTENT_TYPE["xls"] = "application/excel";
	HASH_MIME_CONTENT_TYPE["unv"] = "application/i-deas";
	HASH_MIME_CONTENT_TYPE["jar"] = "application/java-archive";
	HASH_MIME_CONTENT_TYPE["hqx"] = "application/mac-binhex40";
	HASH_MIME_CONTENT_TYPE["cpt"] = " application/mac-compactpro";
	HASH_MIME_CONTENT_TYPE["pot"] = "application/vnd.ms-powerpoint";
	HASH_MIME_CONTENT_TYPE["pps"] = "application/vnd.ms-powerpoint";
	HASH_MIME_CONTENT_TYPE["ppt"] = "application/vnd.ms-powerpoint";
	HASH_MIME_CONTENT_TYPE["ppz"] = "application/vnd.ms-powerpoint";
	HASH_MIME_CONTENT_TYPE["doc"] = "application/msword";
	HASH_MIME_CONTENT_TYPE["bin"] = "application/octet-stream";
	HASH_MIME_CONTENT_TYPE["class"] = "application/octet-stream";
	HASH_MIME_CONTENT_TYPE["dms"] = "application/octet-stream";
	HASH_MIME_CONTENT_TYPE["exe"] = "application/octet-stream";
	HASH_MIME_CONTENT_TYPE["lha"] = "application/octet-stream";
	HASH_MIME_CONTENT_TYPE["lzh"] = "application/octet-stream";
	HASH_MIME_CONTENT_TYPE["oda"] = "application/oda";
	HASH_MIME_CONTENT_TYPE["ogm"] = "application/ogg";
	HASH_MIME_CONTENT_TYPE["pdf"] = "application/pdf";
	HASH_MIME_CONTENT_TYPE["pgp"] = "application/pgp";
	HASH_MIME_CONTENT_TYPE["ai"] = "application/postscript";
	HASH_MIME_CONTENT_TYPE["eps"] = "application/postscript";
	HASH_MIME_CONTENT_TYPE["ps"] = "application/postscript";
	HASH_MIME_CONTENT_TYPE["prt"] = "application/pro_eng";
	HASH_MIME_CONTENT_TYPE["rtf"] = "application/rtf";
	HASH_MIME_CONTENT_TYPE["set"] = "application/set";
	HASH_MIME_CONTENT_TYPE["smi"] = "application/smil";
	HASH_MIME_CONTENT_TYPE["smil"] = "application/smil";
	HASH_MIME_CONTENT_TYPE["mpc"] = "audio/x-musepack"; 
	HASH_MIME_CONTENT_TYPE["txt"] = "text/plain"; 
	HASH_MIME_COMPRESSED["text/plain"] = false;
	HASH_MIME_CONTENT_TYPE["flac"] = "audio/flac"; 
	HASH_MIME_CONTENT_TYPE["xml"] = "text/xml";
	HASH_MIME_COMPRESSED["text/xml"] = false;
	HASH_MIME_CONTENT_TYPE["json"] = "application/json";
	HASH_MIME_COMPRESSED["application/json"] = false;
	HASH_MIME_CONTENT_TYPE["rar"] = "application/x-rar-compressed";
	HASH_MIME_CONTENT_TYPE["zip"] = "application/x-zip-compressed";
	HASH_MIME_CONTENT_TYPE["zip"] = "application/zip";
	HASH_MIME_CONTENT_TYPE["gtar"] = "application/x-gtar";
	HASH_MIME_CONTENT_TYPE["gz"] = "application/x-gunzip";
	HASH_MIME_CONTENT_TYPE["gz"] = "application/x-gzip";
	HASH_MIME_CONTENT_TYPE["iso"] = "application/octet-stream";
	HASH_MIME_CONTENT_TYPE["tif"] = "image/tiff";
	HASH_MIME_COMPRESSED["image/tiff"] = false;
	HASH_MIME_CONTENT_TYPE["tiff"] = "image/tiff";
	HASH_MIME_CONTENT_TYPE["tgz"] = "application/x-tar-gz";
	HASH_MIME_CONTENT_TYPE["tar.gz"] = "application/x-tar-gz";
	HASH_MIME_CONTENT_TYPE["tar"] = "application/x-tar";	
	HASH_MIME_CONTENT_TYPE["mka"] = "audio/x-matroska";
	HASH_MIME_CONTENT_TYPE["mkv"] = "video/x-matroska";
	HASH_MIME_CONTENT_TYPE["sol"] = "application/solids";
	HASH_MIME_CONTENT_TYPE["vda"] = "application/vda";
	HASH_MIME_CONTENT_TYPE["mif"] = "application/vnd.mif";
	HASH_MIME_CONTENT_TYPE["xlc"] = "application/vnd.ms-excel";
	HASH_MIME_CONTENT_TYPE["xll"] = "application/vnd.ms-excel";
	HASH_MIME_CONTENT_TYPE["xlm"] = "application/vnd.ms-excel";
	HASH_MIME_CONTENT_TYPE["xls"] = "application/vnd.ms-excel";
	HASH_MIME_CONTENT_TYPE["xlw"] = "application/vnd.ms-excel";
	HASH_MIME_CONTENT_TYPE["cod"] = "application/vnd.rim.cod";
	HASH_MIME_CONTENT_TYPE["arj"] = "application/x-arj-compressed";
	HASH_MIME_CONTENT_TYPE["bcpio"] = "application/x-bcpio";
	HASH_MIME_CONTENT_TYPE["vcd"] = "application/x-cdlink";
	HASH_MIME_CONTENT_TYPE["pgn"] = "application/x-chess-pgn";
	HASH_MIME_CONTENT_TYPE["cpio"] = "application/x-cpio";
	HASH_MIME_CONTENT_TYPE["csh"] = "application/x-csh";
	HASH_MIME_CONTENT_TYPE["deb"] = "application/x-debian-package";
	HASH_MIME_CONTENT_TYPE["dcr"] = "application/x-director";
	HASH_MIME_CONTENT_TYPE["dir"] = "application/x-director";
	HASH_MIME_CONTENT_TYPE["dxr"] = "application/x-director";
	HASH_MIME_CONTENT_TYPE["dvi"] = "application/x-dvi";
	HASH_MIME_CONTENT_TYPE["pre"] = "application/x-freelance";
	HASH_MIME_CONTENT_TYPE["spl"] = "application/x-futuresplash";
	HASH_MIME_CONTENT_TYPE["hdf"] = "application/x-hdf";
	HASH_MIME_CONTENT_TYPE["ipx"] = "application/x-ipix";
	HASH_MIME_CONTENT_TYPE["ips"] = "application/x-ipscript";
	HASH_MIME_CONTENT_TYPE["skd"] = "application/x-koan";
	HASH_MIME_CONTENT_TYPE["skm"] = "application/x-koan";
	HASH_MIME_CONTENT_TYPE["skp"] = "application/x-koan";
	HASH_MIME_CONTENT_TYPE["skt"] = "application/x-koan";
	HASH_MIME_CONTENT_TYPE["latex"] = "application/x-latex";
	HASH_MIME_CONTENT_TYPE["lsp"] = "application/x-lisp";
	HASH_MIME_CONTENT_TYPE["scm"] = "application/x-lotusscreencam";
	HASH_MIME_CONTENT_TYPE["mif"] = "application/x-mif";
	HASH_MIME_CONTENT_TYPE["bat"] = "application/x-msdos-program";
	HASH_MIME_COMPRESSED["application/x-msdos-program"] = false;
	HASH_MIME_CONTENT_TYPE["com"] = "application/x-msdos-program";
	HASH_MIME_CONTENT_TYPE["exe"] = "application/x-msdos-program";
	HASH_MIME_CONTENT_TYPE["cdf"] = "application/x-netcdf";
	HASH_MIME_CONTENT_TYPE["nc"] = "application/x-netcdf";
	HASH_MIME_CONTENT_TYPE["pl"] = "application/x-perl";
	HASH_MIME_COMPRESSED["application/x-perl"] = false;
	HASH_MIME_CONTENT_TYPE["pm"] = "application/x-perl";
	HASH_MIME_CONTENT_TYPE["sh"] = "application/x-sh";
	HASH_MIME_COMPRESSED["application/x-sh"] = false;
	HASH_MIME_CONTENT_TYPE["shar"] = "application/x-shar";
	HASH_MIME_CONTENT_TYPE["sit"] = "application/x-stuffit";
	HASH_MIME_CONTENT_TYPE["sv4cpio"] = "application/x-sv4cpio";
	HASH_MIME_CONTENT_TYPE["sv4crc"] = "application/x-sv4crc";
	HASH_MIME_CONTENT_TYPE["tcl"] = "application/x-tcl";
	HASH_MIME_CONTENT_TYPE["tex"] = "application/x-tex";
	HASH_MIME_CONTENT_TYPE["texi"] = "application/x-texinfo";
	HASH_MIME_CONTENT_TYPE["texinfo"] = "application/x-texinfo";
	HASH_MIME_CONTENT_TYPE["man"] = "application/x-troff-man";
	HASH_MIME_CONTENT_TYPE["me"] = "application/x-troff-me";
	HASH_MIME_CONTENT_TYPE["ms"] = "application/x-troff-ms";
	HASH_MIME_CONTENT_TYPE["roff"] = "application/x-troff";
	HASH_MIME_CONTENT_TYPE["t"] = "application/x-troff";
	HASH_MIME_CONTENT_TYPE["tr"] = "application/x-troff";
	HASH_MIME_CONTENT_TYPE["ustar"] = "application/x-ustar";
	HASH_MIME_CONTENT_TYPE["src"] = "application/x-wais-source";
	HASH_MIME_CONTENT_TYPE["tsi"] = "audio/TSP-audio";
	HASH_MIME_CONTENT_TYPE["au"] = "audio/basic";
	HASH_MIME_CONTENT_TYPE["snd"] = "audio/basic";
	HASH_MIME_CONTENT_TYPE["kar"] = "audio/midi";
	HASH_MIME_CONTENT_TYPE["mid"] = "audio/midi";
	HASH_MIME_CONTENT_TYPE["midi"] = "audio/midi";
	HASH_MIME_CONTENT_TYPE["mp2"] = "audio/mpeg";
	HASH_MIME_CONTENT_TYPE["mp3"] = "audio/mpeg";
	HASH_MIME_CONTENT_TYPE["mpga"] = "audio/mpeg";
	HASH_MIME_CONTENT_TYPE["au"] = "audio/ulaw";
	HASH_MIME_CONTENT_TYPE["aif"] = "audio/x-aiff";
	HASH_MIME_CONTENT_TYPE["aifc"] = "audio/x-aiff";
	HASH_MIME_CONTENT_TYPE["aiff"] = "audio/x-aiff";
	HASH_MIME_CONTENT_TYPE["wax"] = "audio/x-ms-wax";
	HASH_MIME_CONTENT_TYPE["wma"] = "audio/x-ms-wma";
	HASH_MIME_CONTENT_TYPE["rpm"] = "audio/x-pn-realaudio-plugin";
	HASH_MIME_CONTENT_TYPE["ram"] = "audio/x-pn-realaudio";
	HASH_MIME_CONTENT_TYPE["rm"] = "audio/x-pn-realaudio";
	HASH_MIME_CONTENT_TYPE["ra"] = "audio/x-pn-realaudio";
	HASH_MIME_CONTENT_TYPE["wav"] = "audio/x-wav";
	HASH_MIME_COMPRESSED["audio/x-wav"] = false;
	HASH_MIME_CONTENT_TYPE["pdb"] = "chemical/x-pdb";
	HASH_MIME_CONTENT_TYPE["xyz"] = "chemical/x-pdb";
	HASH_MIME_CONTENT_TYPE["ras"] = "image/cmu-raster";
	HASH_MIME_CONTENT_TYPE["pnm"] = "image/x-portable-anymap";
	HASH_MIME_CONTENT_TYPE["pbm"] = "image/x-portable-bitmap";
	HASH_MIME_CONTENT_TYPE["pgm"] = "image/x-portable-graymap";
	HASH_MIME_CONTENT_TYPE["ppm"] = "image/x-portable-pixmap";
	HASH_MIME_CONTENT_TYPE["rgb"] = "image/x-rgb";
	HASH_MIME_CONTENT_TYPE["xbm"] = "image/x-xbitmap";
	HASH_MIME_CONTENT_TYPE["xwd"] = "image/x-xwindowdump";
	HASH_MIME_CONTENT_TYPE["iges"] = "model/iges";
	HASH_MIME_CONTENT_TYPE["igs"] = "model/iges";
	HASH_MIME_CONTENT_TYPE["mesh"] = "model/mesh";
	HASH_MIME_CONTENT_TYPE["msh"] = "model/mesh";
	HASH_MIME_CONTENT_TYPE["silo"] = "model/mesh";
	HASH_MIME_CONTENT_TYPE["vrml"] = "model/vrml";
	HASH_MIME_CONTENT_TYPE["wrl"] = "model/vrml";
	HASH_MIME_CONTENT_TYPE["asc"] = "text/plain";
	HASH_MIME_COMPRESSED["text/plain"] = false;
	HASH_MIME_CONTENT_TYPE["c"] = "text/plain";
	HASH_MIME_CONTENT_TYPE["cc"] = "text/plain";
	HASH_MIME_CONTENT_TYPE["cpp"] = "text/plain";
	HASH_MIME_CONTENT_TYPE["f90"] = "text/plain";
	HASH_MIME_CONTENT_TYPE["f"] = "text/plain";
	HASH_MIME_CONTENT_TYPE["h"] = "text/plain";
	HASH_MIME_CONTENT_TYPE["hh"] = "text/plain";
	HASH_MIME_CONTENT_TYPE["m"] = "text/plain";
	HASH_MIME_CONTENT_TYPE["rtx"] = "text/richtext";
	HASH_MIME_COMPRESSED["text/richtext"] = false;
	HASH_MIME_CONTENT_TYPE["rtf"] = "text/rtf";
	HASH_MIME_COMPRESSED["text/rtf"] = false;
	HASH_MIME_CONTENT_TYPE["sgm"] = "text/sgml";
	HASH_MIME_CONTENT_TYPE["sgml"] = "text/sgml";
	HASH_MIME_CONTENT_TYPE["tsv"] = "text/tab-separated-values";
	HASH_MIME_CONTENT_TYPE["jad"] = "text/vnd.sun.j2me.app-descriptor";
	HASH_MIME_CONTENT_TYPE["etx"] = "text/x-setext";
	HASH_MIME_CONTENT_TYPE["dl"] = "video/dl";
	HASH_MIME_CONTENT_TYPE["fli"] = "video/fli";
	HASH_MIME_CONTENT_TYPE["gl"] = "video/gl";
	HASH_MIME_CONTENT_TYPE["movie"] = "video/x-sgi-movie";
	HASH_MIME_CONTENT_TYPE["mime"] = "www/mime";
	HASH_MIME_CONTENT_TYPE["ice"] = "x-conference/x-cooltalk";
	HASH_MIME_CONTENT_TYPE["vrm"] = " x-world/x-vrml";
	HASH_MIME_CONTENT_TYPE["vrml"] = " x-world/x-vrml";

	HASH_MIME_CONTENT_TYPE["form"] = "application/x-www-form-urlencoded";
	HASH_MIME_COMPRESSED["application/x-www-form-urlencoded"] = false;

	HashMimeContentTypeInitialized = true;
};

const QString NMimeType_n::fileSuffixToMIME(const QString & suffix)
{
	initHashMimeContentType();
	QString lowerSuffix = suffix.toLower();
	if (!HASH_MIME_CONTENT_TYPE.contains(lowerSuffix))
		NLOGM("Not managed mime type", lowerSuffix);
	return HASH_MIME_CONTENT_TYPE.value(lowerSuffix, "application/octet-stream");
}

bool NMimeType_n::contentTypeNeedCompression(const QString & contentType)
{
	initHashMimeContentType();
	return !HASH_MIME_COMPRESSED.value(contentType.toLower(), true);
}
