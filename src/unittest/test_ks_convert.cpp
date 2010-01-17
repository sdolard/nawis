// Qt
#include <QtTest/QtTest>
     
#include "../ks_convert.h"

class TestKSConvert: public QObject
{
    Q_OBJECT
private slots:
    void byteToHuman();
	void durationToHuman();
	void rateForOneSecToHuman();
};

void TestKSConvert::byteToHuman()
{     
	const qint64 KB = 1024;
	const qint64 MB = 1024 * KB;
	const qint64 GB = 1024 * MB;
	const qint64 TB = 1024 * GB;
	
	// Byte                                              
    QCOMPARE(KSConvert_n::byteToHuman(0), QString("0B"));                       
	QCOMPARE(KSConvert_n::byteToHuman(1023), QString("1023B"));    
	       
	// KByte
	QCOMPARE(KSConvert_n::byteToHuman(KB), QString("1KB"));  
	QCOMPARE(KSConvert_n::byteToHuman(KB + 1), QString("1KB"));
	QCOMPARE(KSConvert_n::byteToHuman(KB + 128), QString("1.1KB")); 
	QCOMPARE(KSConvert_n::byteToHuman(KB * 2 - 128 ), QString("1.9KB"));                                                    
	QCOMPARE(KSConvert_n::byteToHuman(KB * 2 - 1 ), QString("2KB"));
	QCOMPARE(KSConvert_n::byteToHuman(MB - 128), QString("1023.9KB"));
	QCOMPARE(KSConvert_n::byteToHuman(MB - 1), QString("1024KB")); 
	                                                          
	// MByte
	QCOMPARE(KSConvert_n::byteToHuman(MB), QString("1MB"));  
	QCOMPARE(KSConvert_n::byteToHuman(MB + 1 * KB), QString("1MB"));
	QCOMPARE(KSConvert_n::byteToHuman(MB + 128 * KB), QString("1.1MB")); 
	QCOMPARE(KSConvert_n::byteToHuman(MB * 2 - 128 * KB), QString("1.9MB"));                                                    
	QCOMPARE(KSConvert_n::byteToHuman(MB * 2 - 1 * KB ), QString("2MB"));
	QCOMPARE(KSConvert_n::byteToHuman(GB - 128 * KB), QString("1023.9MB"));
	QCOMPARE(KSConvert_n::byteToHuman(GB - 1 * KB), QString("1024MB")); 

	// GByte
	QCOMPARE(KSConvert_n::byteToHuman(GB), QString("1GB"));  
	QCOMPARE(KSConvert_n::byteToHuman(GB + 1 * MB), QString("1GB"));
	QCOMPARE(KSConvert_n::byteToHuman(GB + 128 * MB), QString("1.1GB")); 
	QCOMPARE(KSConvert_n::byteToHuman(GB * 2 - 128 * MB), QString("1.9GB"));                                                    
	QCOMPARE(KSConvert_n::byteToHuman(GB * 2 - 1 * MB ), QString("2GB"));
	QCOMPARE(KSConvert_n::byteToHuman(TB - 128 * MB), QString("1023.9GB"));
	QCOMPARE(KSConvert_n::byteToHuman(TB - 1 * MB), QString("1024GB")); 
	
	// TByte
	QCOMPARE(KSConvert_n::byteToHuman(TB), QString("1TB"));  
	QCOMPARE(KSConvert_n::byteToHuman(TB + 1 * GB), QString("1TB"));
	QCOMPARE(KSConvert_n::byteToHuman(TB + 128 * GB), QString("1.1TB")); 
	QCOMPARE(KSConvert_n::byteToHuman(TB * 2 - 128 * GB), QString("1.9TB"));                                                    
	QCOMPARE(KSConvert_n::byteToHuman(TB * 2 - 1 * GB ), QString("2TB"));
	QCOMPARE(KSConvert_n::byteToHuman(TB * 1024 - 128 * GB), QString("1023.9TB"));
	QCOMPARE(KSConvert_n::byteToHuman(TB * 1024 - 1 * GB), QString("1024TB")); 
}

void TestKSConvert::durationToHuman()
{
	const qint64 OneSec = 1000;
	const qint64 OneMin = OneSec * 60;
	const qint64 OneHour = OneMin * 60;
	const qint64 OneDay = OneHour * 24;
	
	QCOMPARE(KSConvert_n::durationToHuman(0), QString("0ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneSec - 1), QString("999ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneSec), QString("1s"));
	QCOMPARE(KSConvert_n::durationToHuman(OneSec + 999), QString("1s 999ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneMin - 1), QString("59s 999ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneMin), QString("1m"));
	QCOMPARE(KSConvert_n::durationToHuman(OneMin + OneSec), QString("1m 1s"));
	QCOMPARE(KSConvert_n::durationToHuman(OneMin + OneSec + 1), QString("1m 1s 1ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneMin + 59 * OneSec + 999), QString("1m 59s 999ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneHour - 1), QString("59m 59s 999ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneHour), QString("1h"));
	QCOMPARE(KSConvert_n::durationToHuman(OneHour + OneMin), QString("1h 1m"));
	QCOMPARE(KSConvert_n::durationToHuman(OneHour + OneMin + OneSec), QString("1h 1m 1s"));
	QCOMPARE(KSConvert_n::durationToHuman(OneHour + OneMin + OneSec + 1), QString("1h 1m 1s 1ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneHour + 59 * OneMin + 59 * OneSec + 999), QString("1h 59m 59s 999ms"));
	QCOMPARE(KSConvert_n::durationToHuman(23 * OneHour + 59 * OneMin + 59 * OneSec + 999), QString("23h 59m 59s 999ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneDay - 1), QString("23h 59m 59s 999ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneDay), QString("1d"));
	QCOMPARE(KSConvert_n::durationToHuman(OneDay + OneHour), QString("1d 1h"));
	QCOMPARE(KSConvert_n::durationToHuman(OneDay + OneHour + OneMin), QString("1d 1h 1m"));
	QCOMPARE(KSConvert_n::durationToHuman(OneDay + OneHour + OneMin + OneSec), QString("1d 1h 1m 1s"));
	QCOMPARE(KSConvert_n::durationToHuman(OneDay + OneHour + OneMin + OneSec + 1), QString("1d 1h 1m 1s 1ms"));
	QCOMPARE(KSConvert_n::durationToHuman(OneDay + 23 * OneHour + 59 * OneMin + 59 * OneSec + 999), QString("1d 23h 59m 59s 999ms"));
	QCOMPARE(KSConvert_n::durationToHuman(100 * OneDay - 1), QString("99d 23h 59m 59s 999ms"));
	
}


void TestKSConvert::rateForOneSecToHuman()
{
	const qint64 KB = 1024;
	const qint64 OneSec = 1000;
	const qint64 OneMin = OneSec * 60;
	
	QCOMPARE(KSConvert_n::rateForOneSecToHuman(KB, OneSec), QString("1KB/s"));
	QCOMPARE(KSConvert_n::rateForOneSecToHuman(KB, OneMin), QString("17B/s"));
}

QTEST_MAIN(TestKSConvert)
#include "test_ks_convert.moc"