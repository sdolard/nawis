// Qt
#include <QtTest/QtTest>

#include "../ks_settings.h"

class TestKSSettings: public QObject
	{
		Q_OBJECT
		private slots:              
		void KSSCommandContrutors(); 
		void toKSSCommand();    
	};

void TestKSSettings::KSSCommandContrutors()
{    
	// invalid
	KSSCommand cmd = KSSCommand("");  
	QCOMPARE(cmd.isValid(), false);
	
	// Comment
    cmd = KSSCommand(" "); 
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.type(), KSSCommand::ktComment);   
	QCOMPARE(cmd.toString(), QString("#  "));         
    
	// ListItem
	cmd = KSSCommand("context", KSSCommand::kaDef, "toto");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.toString(), QString("context def toto")); 
	
	cmd = KSSCommand("context", KSSCommand::kaDef, "toto", QVariant(), "comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.toString(), QString("context def toto # comment")); 
	
	cmd = KSSCommand("context", KSSCommand::kaUndef, "toto");
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);  
	QCOMPARE(cmd.toString(), QString("context undef toto"));
   	
	cmd = KSSCommand("context", KSSCommand::kaDef, "toto", 1);
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.toString(), QString("context def toto=1"));
	
	cmd = KSSCommand("context", KSSCommand::kaDef, "toto", QVariant(), "comment");
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.type(), KSSCommand::ktListItem); 
	QCOMPARE(cmd.toString(), QString("context def toto # comment"));
	
	cmd = KSSCommand("context", KSSCommand::kaDef, "toto", 2, "comment");
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.type(), KSSCommand::ktListItem); 
	QCOMPARE(cmd.toString(), QString("context def toto=2 # comment"));
	
	
	// ctGroupItem
	cmd = KSSCommand("context", 0, KSSCommand::kaDef, "toto");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.toString(), QString("context group 0 def toto")); 
	
	cmd = KSSCommand("context", 0, KSSCommand::kaDef, "toto", QVariant(), "comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.toString(), QString("context group 0 def toto # comment")); 
	
	cmd = KSSCommand("context", 0, KSSCommand::kaUndef, "toto");
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);  
	QCOMPARE(cmd.toString(), QString("context group 0 undef toto"));
   	
	cmd = KSSCommand("context", 0, KSSCommand::kaDef, "toto", 1);
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.toString(), QString("context group 0 def toto=1"));
	
	cmd = KSSCommand("context", 0, KSSCommand::kaDef, "toto", QVariant(), "comment");
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem); 
	QCOMPARE(cmd.toString(), QString("context group 0 def toto # comment"));
	
	cmd = KSSCommand("context", 0, KSSCommand::kaDef, "toto", 2, "comment");
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem); 
	QCOMPARE(cmd.toString(), QString("context group 0 def toto=2 # comment"));
}      

void TestKSSettings::toKSSCommand()
{     
	// Invalid
	KSSCommand cmd = KSSCommand::toKSSCommand("");  
	QCOMPARE(cmd.isValid(), false);
	
	// Comment
	cmd = KSSCommand::toKSSCommand("#");  
	QCOMPARE(cmd.isValid(), false);
	
	cmd = KSSCommand::toKSSCommand("# comment");  
	QCOMPARE(cmd.isValid(), true);
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.type(), KSSCommand::ktComment); 
	
	// ListItem
	cmd = KSSCommand::toKSSCommand("context  def  toto");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("context"));
	QCOMPARE(cmd.key(), QString("toto"));
	QCOMPARE(cmd.toString(), QString("context def toto"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" def toto");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("toto"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" def toto"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" def \"to to\"");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" def \"to to\""));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" def  \"to to\" # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" def \"to to\" # comment"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" undef \"to to\" # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.action(), KSSCommand::kaUndef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" undef \"to to\" # comment"));
	
	cmd = KSSCommand::toKSSCommand("context def toto=1");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("context"));
	QCOMPARE(cmd.key(), QString("toto"));
	QCOMPARE(cmd.value().toString(), QString("1"));
	QCOMPARE(cmd.comment().isEmpty(), true);
	QCOMPARE(cmd.toString(), QString("context def toto=1"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" undef toto=1 # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.action(), KSSCommand::kaUndef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("toto"));
	QCOMPARE(cmd.value().toString(), QString("1"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" undef toto=1 # comment"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" undef \"to to\"=1 # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.action(), KSSCommand::kaUndef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.value().toString(), QString("1"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" undef \"to to\"=1 # comment"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" undef \"to to\" = \"1 2\" # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktListItem);
	QCOMPARE(cmd.action(), KSSCommand::kaUndef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.value().toString(), QString("1 2"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" undef \"to to\"=\"1 2\" # comment"));
	
	// GroupItem
	cmd = KSSCommand::toKSSCommand("context group 0 def toto");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("context"));
	QCOMPARE(cmd.key(), QString("toto"));
	QCOMPARE(cmd.toString(), QString("context group 0 def toto"));
	
	cmd = KSSCommand::toKSSCommand("context group 0 undef");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaUndef);
	QCOMPARE(cmd.context(), QString("context"));
	QCOMPARE(cmd.key().isEmpty(), true);
	QCOMPARE(cmd.toString(), QString("context group 0 undef"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" group 0 def toto");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("toto"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" group 0 def toto"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" group 0 def \"to to\"");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" group 0 def \"to to\""));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" group 0 def \"to to\" # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" group 0 def \"to to\" # comment"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" group 0 undef \"to to\" # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaUndef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" group 0 undef \"to to\" # comment"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" group 0 undef toto=1 # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaUndef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("toto"));
	QCOMPARE(cmd.value().toString(), QString("1"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" group 0 undef toto=1 # comment"));
	
	cmd = KSSCommand::toKSSCommand("context group 0 def toto=1");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaDef);
	QCOMPARE(cmd.context(), QString("context"));
	QCOMPARE(cmd.key(), QString("toto"));
	QCOMPARE(cmd.value().toString(), QString("1"));
	QCOMPARE(cmd.comment().isEmpty(), true);
	QCOMPARE(cmd.toString(), QString("context group 0 def toto=1"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" group 0 undef \"to to\"=1 # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaUndef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.value().toString(), QString("1"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" group 0 undef \"to to\"=1 # comment"));
	
	cmd = KSSCommand::toKSSCommand("\"co ntext\" group 0 undef \"to to\"=\"1 2\" # comment");
	QCOMPARE(cmd.isValid(), true); 
	QCOMPARE(cmd.type(), KSSCommand::ktGroupItem);
	QCOMPARE(cmd.group(), 0);
	QCOMPARE(cmd.action(), KSSCommand::kaUndef);
	QCOMPARE(cmd.context(), QString("co ntext"));
	QCOMPARE(cmd.key(), QString("to to"));
	QCOMPARE(cmd.value().toString(), QString("1 2"));
	QCOMPARE(cmd.comment(), QString("comment"));
	QCOMPARE(cmd.toString(), QString("\"co ntext\" group 0 undef \"to to\"=\"1 2\" # comment"));
	
} 


QTEST_MAIN(TestKSSettings)
#include "test_ks_settings.moc"         
