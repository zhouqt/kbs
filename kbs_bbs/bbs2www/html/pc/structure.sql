# smth.blog
# dec 21 , 2003 

CREATE TABLE `comments` ( 
  `cid` int(10) unsigned NOT NULL auto_increment, 
  `nid` int(10) unsigned default NULL, 
  `uid` int(10) default NULL, 
  `hostname` varchar(20) NOT NULL default '', 
  `username` varchar(20) NOT NULL default '', 
  `subject` varchar(200) NOT NULL default 'null', 
  `created` timestamp(14) NOT NULL, 
  `changed` timestamp(14) NOT NULL, 
  `body` text, 
  `emote` int(2) NOT NULL default '0', 
  `htmltag` int(1) NOT NULL default '0', 
  PRIMARY KEY  (`cid`), 
  KEY `nid` (`nid`), 
  KEY `created` (`created`), 
  KEY `changed` (`changed`), 
  FULLTEXT KEY `body` (`body`) 
) TYPE=MyISAM COMMENT='评论表' AUTO_INCREMENT=15 ; 

CREATE TABLE `directories` ( 
  `did` int(10) unsigned NOT NULL auto_increment, 
  `pid` int(10) unsigned NOT NULL default '0', 
  `subject` varchar(200) NOT NULL default '', 
  `uid` int(10) NOT NULL default '0', 
  `created` timestamp(14) NOT NULL, 
  `CHANGED` timestamp(14) NOT NULL, 
  `hostname` varchar(20) NOT NULL default '', 
  PRIMARY KEY  (`did`) 
) TYPE=MyISAM COMMENT='用户表' AUTO_INCREMENT=1 ; 

CREATE TABLE `logs` ( 
  `lid` int(10) unsigned NOT NULL auto_increment, 
  `username` varchar(20) binary NOT NULL default '', 
  `hostname` varchar(20) binary NOT NULL default '', 
  `ACTION` varchar(20) NOT NULL default '', 
  `pri_id` int(10) unsigned default NULL, 
  `sec_id` int(10) unsigned default NULL, 
  `COMMENT` varchar(200) default NULL, 
  `logtime` timestamp(14) NOT NULL, 
  PRIMARY KEY  (`lid`), 
  KEY `username` (`username`,`hostname`,`ACTION`,`pri_id`,`sec_id`,`logtime`) 
) TYPE=MyISAM COMMENT='日志表' AUTO_INCREMENT=1 ; 

CREATE TABLE `nodes` ( 
  `nid` int(10) unsigned NOT NULL auto_increment, 
  `pid` int(10) unsigned default NULL, 
  `type` int(4) NOT NULL default '0', 
  `source` varchar(10) NOT NULL default '', 
  `hostname` varchar(20) NOT NULL default '', 
  `changed` timestamp(14) NOT NULL, 
  `created` timestamp(14) NOT NULL, 
  `uid` int(10) default NULL, 
  `comment` int(4) NOT NULL default '0', 
  `commentcount` int(10) NOT NULL default '0', 
  `subject` varchar(200) NOT NULL default 'null', 
  `body` text, 
  `access` int(4) NOT NULL default '0', 
  `visitcount` int(4) NOT NULL default '0', 
  `tid` int(10) NOT NULL default '0', 
  `emote` int(2) NOT NULL default '0', 
  `htmltag` int(1) NOT NULL default '0', 
  `trackback` int(1) NOT NULL default '1', 
  `trackbackcount` int(10) NOT NULL default '0', 
  PRIMARY KEY  (`nid`), 
  KEY `pid` (`pid`), 
  KEY `created` (`changed`), 
  KEY `changed` (`created`), 
  FULLTEXT KEY `body` (`body`) 
) TYPE=MyISAM COMMENT='文章表' AUTO_INCREMENT=85 ; 

CREATE TABLE `topics` ( 
  `tid` int(10) NOT NULL auto_increment, 
  `uid` int(10) NOT NULL default '0', 
  `access` int(1) NOT NULL default '0', 
  `topicname` varchar(200) NOT NULL default '', 
  `sequen` int(2) NOT NULL default '0', 
  KEY `tid` (`tid`) 
) TYPE=MyISAM COMMENT='用户文集分类' AUTO_INCREMENT=9 ; 

CREATE TABLE `trackback` ( 
  `tbid` int(10) unsigned NOT NULL auto_increment, 
  `uid` int(10) unsigned NOT NULL default '0', 
  `nid` int(10) unsigned NOT NULL default '0', 
  `title` varchar(50) NOT NULL default '', 
  `excerpt` varchar(255) NOT NULL default '', 
  `url` varchar(255) NOT NULL default '', 
  `blogname` varchar(50) NOT NULL default '', 
  `time` timestamp(14) NOT NULL, 
  `address` varchar(15) NOT NULL default '', 
  PRIMARY KEY  (`tbid`) 
) TYPE=MyISAM COMMENT='引用通告' AUTO_INCREMENT=10 ; 

CREATE TABLE `users` ( 
  `uid` int(10) unsigned NOT NULL auto_increment, 
  `username` varchar(20) NOT NULL default '', 
  `corpusname` varchar(40) NOT NULL default '', 
  `description` varchar(200) default NULL, 
  `theme` varchar(20) default NULL, 
  `nodelimit` int(8) NOT NULL default '300', 
  `dirlimit` int(8) NOT NULL default '100', 
  `createtime` timestamp(14) NOT NULL, 
  `style` int(2) NOT NULL default '0', 
  `backimage` varchar(255) default NULL, 
  `visitcount` int(10) NOT NULL default '0', 
  `nodescount` int(10) NOT NULL default '0', 
  `logoimage` varchar(255) default NULL, 
  `modifytime` timestamp(14) NOT NULL, 
  `calendar` text NOT NULL, 
  `links` varchar(255) NOT NULL default '', 
  `htmleditor` int(1) NOT NULL default '0', 
  PRIMARY KEY  (`uid`), 
  UNIQUE KEY `username` (`username`), 
  KEY `corpusname` (`corpusname`,`createtime`), 
  FULLTEXT KEY `corpusname_2` (`corpusname`,`description`) 
) TYPE=MyISAM COMMENT='用户表' AUTO_INCREMENT=2 ; 
