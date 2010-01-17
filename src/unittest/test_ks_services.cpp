// Qt
#include <QtTest/QtTest>

#include "../ks_services.h"

class TestKSServices: public QObject
{
	Q_OBJECT
		private slots:
	void toService();
};

void TestKSServices::toService()
{                     
	QStringList path;         
	KSService_n::KSService service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_NONE);

	path << "cmd";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD);                   
	QCOMPARE(service.comment, QString("Display root commands")); 
	QCOMPARE(service.params, QString("[help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd"));
	QCOMPARE(service.returns, QString("Command help"));  

	path.clear();
	path << "cmd" << "file";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_FILE);
	QCOMPARE(service.comment, QString("File root commands")); 
	QCOMPARE(service.params, QString("[help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/file"));
	QCOMPARE(service.returns, QString("File command help")); 

	path.clear();
	path << "cmd" << "file" << "categories";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_FILE_CATEGORIES); 
	QCOMPARE(service.comment, QString("List file category type")); 
	QCOMPARE(service.params, QString("[help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/file/categories"));
	QCOMPARE(service.returns, QString("<categories>" \
		"<category>All</category>" \
		"<category>Archive</category>" \
		"<category>Document</category>" \
		"<category>Music</category>" \
		"<category>Other</category>" \
		"<category>Picture</category>" \
		"<category>Video</category>" \
		"</categories>")); 

	path.clear();
	path << "cmd" << "file" << "count";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_FILE_COUNT);
	QCOMPARE(service.comment, QString("Shared file number")); 
	QCOMPARE(service.params, QString("[search=<string>[+<string>+...]][category=<string>][help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/file/count"));
	QCOMPARE(service.returns, QString("<count>12</count>"));   

	path.clear();
	path << "cmd" << "file" << "dl";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_FILE_DOWNLOAD);
	QCOMPARE(service.comment, QString("Download a file")); 
	QCOMPARE(service.params, QString("hash=<string>[help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/file/dl?hash=9f26c647c80745f66313b1b6c6a31c8da2af2874"));
	QCOMPARE(service.returns, QString("")); 

	path.clear();
	path << "cmd" << "file" << "list";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_FILE_LIST);  
	QCOMPARE(service.comment, QString("List shared files (limited: default list is limited to 25 items per page)")); 
	QCOMPARE(service.params, QString("[search=<string>[+<string>+...]][category=<string>][start=<int>][limit=<int>][sort=<string>][dir=ASC|DESC][help]"));
	QCOMPARE(service.example, QString("http://localhost:6391/cmd/file/list?search=holidays+2008&limit=1&sort=fileName&dir=ASC"));
	QCOMPARE(service.returns, QString("<lines>"\
		"<line>"\
		"<added>2008-08-02 17:41:00.284</added>"\
		"<category>Picture</category>"\
		"<fileName>8543.jpg</fileName>"\
		"<hash>eaa7e6b7d5326cc52e81c44ca8b182c5f7a40e00</hash>"\
		"<relativePath>/Pictures/holidays/2008/Italia</relativePath>"\
		"<size>1071296</size>"\
		"</line>"\
		"</lines>")); 

	path.clear();
	path << "cmd" << "dl";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_DL);
	QCOMPARE(service.comment, QString("Download root commands")); 
	QCOMPARE(service.params, QString("[help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/dl"));
	QCOMPARE(service.returns, QString("Download command help")); 

	path.clear();
	path << "cmd" << "dl" << "add";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_DL_ADD);
	QCOMPARE(service.comment, QString("Enqueue a file to download")); 
	QCOMPARE(service.params, QString("hash=<string>[place=<int>]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/dl/add?hash=eaa7e6b7d5326cc52e81c44ca8b182c5f7a40e00"));
	QCOMPARE(service.returns, QString(""));  

	path.clear();
	path << "cmd" << "dl" << "del";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_DL_DELETE);
	QCOMPARE(service.comment, QString("Delete a file from download queue")); 
	QCOMPARE(service.params, QString("hash=<string>[help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/dl/delete?hash=eaa7e6b7d5326cc52e81c44ca8b182c5f7a40e00"));
	QCOMPARE(service.returns, QString(""));

	path.clear();
	path << "cmd" << "dl" << "list";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_DL_LIST);
	QCOMPARE(service.comment, QString("List download file")); 
	QCOMPARE(service.params, QString("[help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/dl/list"));
	QCOMPARE(service.returns, QString("")); 

	path.clear();
	path << "cmd" << "log";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_LOG);  
	QCOMPARE(service.comment, QString("Log root commands")); 
	QCOMPARE(service.params, QString("[help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/log"));
	QCOMPARE(service.returns, QString("Log command help")); 

	path.clear();
	path << "cmd" << "log" << "del";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_LOG_DELETE);
	QCOMPARE(service.comment, QString("Delete logs")); 
	QCOMPARE(service.params, QString("[help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/log/del"));
	QCOMPARE(service.returns, QString(""));

	path.clear();
	path << "cmd" << "log" << "list";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_CMD_LOG_LIST);
	QCOMPARE(service.comment, QString("List logs")); 
	QCOMPARE(service.params, QString("[search=<string>[+<string>+...]][start=<int>][limit=<int>][sort=ASC|DESC][dir=<string>][help]"));
	QCOMPARE(service.example, QString("http://localhost:<nawis port>/cmd/log/list?limit=1&sort=date&dir=DESC"));
	QCOMPARE(service.returns, QString("<lines>"\
		"<line>"\
		"<date>2008-11-02 18:55:08.772</date>"\
		"<log>KSServer: Shared: 22.8GB</log>"\
		"</line>"\
		"</lines>"));   

	path.clear();
	path << "favicon.ico";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_FAVICON); 

	path.clear();
	path << "help";
	service = KSService_n::toService(KSService_n::kssRootServices, path); 
	QCOMPARE(service.id, SVC_HELP);
}


QTEST_MAIN(TestKSServices)
#include "test_ks_services.moc"