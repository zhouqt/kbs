--
-- Table structure for table `blacklist`
--

CREATE TABLE `blacklist` (
  `userid` varchar(12) NOT NULL default '',
  `uid` int(10) unsigned NOT NULL default '0',
  `manager` varchar(12) NOT NULL default '',
  `hostname` varchar(20) NOT NULL default '',
  `addtime` timestamp(14) NOT NULL,
  KEY `userid` (`userid`,`uid`)
) TYPE=MyISAM COMMENT='blog黑名单';

--
-- Table structure for table `comments`
--

CREATE TABLE `comments` (
  `cid` int(10) unsigned NOT NULL auto_increment,
  `nid` int(10) unsigned default NULL,
  `uid` int(10) default NULL,
  `hostname` varchar(20) NOT NULL default '',
  `username` varchar(20) NOT NULL default '',
  `subject` varchar(200) NOT NULL default 'null',
  `created` timestamp(14) NOT NULL,
  `changed` timestamp(14) NOT NULL default '00000000000000',
  `body` text,
  `emote` int(2) NOT NULL default '0',
  `htmltag` int(1) NOT NULL default '0',
  PRIMARY KEY  (`cid`),
  KEY `nid` (`nid`),
  KEY `created` (`created`),
  KEY `changed` (`changed`),
  KEY `uid` (`uid`)
) TYPE=MyISAM COMMENT='评论表';

--
-- Table structure for table `filter`
--

CREATE TABLE `filter` (
  `fid` int(10) unsigned NOT NULL auto_increment,
  `nid` int(10) NOT NULL default '0',
  `pid` int(10) unsigned default NULL,
  `type` int(4) NOT NULL default '0',
  `state` int(1) NOT NULL default '0',
  `source` varchar(10) NOT NULL default '',
  `hostname` varchar(20) NOT NULL default '',
  `changed` timestamp(14) NOT NULL,
  `created` timestamp(14) NOT NULL default '00000000000000',
  `uid` int(10) default NULL,
  `username` varchar(13) NOT NULL default '',
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
  `recommend` int(1) NOT NULL default '0',
  `updatetime` timestamp(14) NOT NULL default '00000000000000',
  `nodetype` int(1) NOT NULL default '0',
  `recuser` varchar(12) NOT NULL default '',
  `tbp_url` varchar(255) NOT NULL default '',
  `tbp_art` varchar(255) NOT NULL default '',
  `auto_tbp` int(1) NOT NULL default '0',
  `tbpencoding` varchar(20) NOT NULL default '',
  `theme` varchar(20) NOT NULL default '',
  `publisher` varchar(14) NOT NULL default '',
  PRIMARY KEY  (`fid`),
  KEY `pid` (`pid`),
  KEY `created` (`changed`),
  KEY `changed` (`created`),
  KEY `state` (`state`),
  FULLTEXT KEY `body` (`body`),
  FULLTEXT KEY `publisher` (`publisher`)
) TYPE=MyISAM COMMENT='过滤器';

--
-- Table structure for table `logs`
--

CREATE TABLE `logs` (
  `lid` int(10) unsigned NOT NULL auto_increment,
  `username` varchar(20) NOT NULL default '',
  `hostname` varchar(20) NOT NULL default '',
  `ACTION` varchar(100) NOT NULL default '',
  `pri_id` varchar(20) default NULL,
  `sec_id` varchar(20) default NULL,
  `COMMENT` varchar(200) default NULL,
  `logtime` timestamp(14) NOT NULL,
  PRIMARY KEY  (`lid`),
  KEY `username` (`username`,`hostname`,`ACTION`,`pri_id`,`sec_id`,`logtime`),
  KEY `logtime` (`logtime`),
  KEY `pri_id` (`pri_id`)
) TYPE=MyISAM COMMENT='日志表';

--
-- Table structure for table `members`
--

CREATE TABLE `members` (
  `uid` int(10) unsigned NOT NULL default '0',
  `username` varchar(12) NOT NULL default '',
  KEY `uid` (`uid`)
) TYPE=MyISAM COMMENT='公有blog成员表';

--
-- Table structure for table `newapply`
--

CREATE TABLE `newapply` (
  `naid` int(10) NOT NULL auto_increment,
  `username` varchar(12) NOT NULL default '',
  `appname` varchar(50) NOT NULL default '',
  `appself` text NOT NULL,
  `appdirect` text NOT NULL,
  `hostname` varchar(15) NOT NULL default '',
  `apptime` timestamp(14) NOT NULL,
  `manager` varchar(12) default NULL,
  `management` int(1) unsigned NOT NULL default '1',
  UNIQUE KEY `naid_2` (`naid`),
  KEY `naid` (`naid`),
  KEY `username` (`username`),
  KEY `management` (`management`)
) TYPE=MyISAM COMMENT='新申请用户';

--
-- Table structure for table `nodes`
--

CREATE TABLE `nodes` (
  `nid` int(10) unsigned NOT NULL auto_increment,
  `pid` int(10) unsigned default NULL,
  `type` int(4) NOT NULL default '0',
  `source` varchar(10) NOT NULL default '',
  `hostname` varchar(20) NOT NULL default '',
  `changed` timestamp(14) NOT NULL,
  `created` timestamp(14) NOT NULL default '00000000000000',
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
  `recommend` int(1) NOT NULL default '0',
  `updatetime` timestamp(14) NOT NULL default '00000000000000',
  `nodetype` int(1) NOT NULL default '0',
  `recuser` varchar(12) NOT NULL default '',
  `rectopic` varchar(20) NOT NULL default '0',
  `theme` varchar(20) NOT NULL default 'others',
  `publisher` varchar(14) NOT NULL default '',
  PRIMARY KEY  (`nid`),
  KEY `pid` (`pid`),
  KEY `nodetype` (`nodetype`),
  KEY `rectopic` (`rectopic`),
  KEY `commentcount` (`commentcount`),
  KEY `theme` (`theme`),
  KEY `uid` (`uid`),
  KEY `access` (`access`),
  KEY `type` (`type`),
  KEY `comment` (`comment`),
  KEY `visitcount` (`visitcount`),
  KEY `recommend` (`recommend`),
  KEY `tid` (`tid`),
  KEY `changed` (`changed`),
  KEY `created` (`created`),
  KEY `uid_nid` (`uid`,`nid`),
  FULLTEXT KEY `publisher` (`publisher`)
) TYPE=MyISAM COMMENT='文章表';

--
-- Table structure for table `recommend`
--

CREATE TABLE `recommend` (
  `rid` int(10) unsigned NOT NULL auto_increment,
  `nid` int(10) unsigned NOT NULL default '0',
  `hostname` varchar(20) NOT NULL default '',
  `created` timestamp(14) NOT NULL,
  `uid` int(10) default NULL,
  `subject` varchar(200) NOT NULL default 'null',
  `body` text,
  `emote` int(2) NOT NULL default '0',
  `htmltag` int(1) NOT NULL default '0',
  `recuser` varchar(12) NOT NULL default '',
  `state` int(1) NOT NULL default '1',
  `topic` varchar(20) NOT NULL default '0',
  PRIMARY KEY  (`rid`),
  KEY `rid` (`rid`),
  KEY `topic` (`topic`),
  KEY `state_rid` (`state`,`rid`),
  FULLTEXT KEY `body` (`body`)
) TYPE=MyISAM COMMENT='推荐文章';

--
-- Table structure for table `topics`
--

CREATE TABLE `topics` (
  `tid` int(10) NOT NULL auto_increment,
  `uid` int(10) NOT NULL default '0',
  `access` int(1) NOT NULL default '0',
  `topicname` varchar(200) NOT NULL default '',
  `sequen` int(2) NOT NULL default '0',
  KEY `tid` (`tid`),
  KEY `uid` (`uid`),
  KEY `access` (`access`),
  KEY `tid_uid_access` (`tid`,`uid`,`access`)
) TYPE=MyISAM COMMENT='用户文集分类';

--
-- Table structure for table `trackback`
--

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
) TYPE=MyISAM COMMENT='引用通告';

--
-- Table structure for table `userfiles`
--

CREATE TABLE `userfiles` (
  `fid` int(10) unsigned NOT NULL auto_increment,
  `uid` int(10) unsigned NOT NULL default '0',
  `pid` int(10) unsigned NOT NULL default '0',
  `type` int(1) unsigned NOT NULL default '0',
  `filename` varchar(200) NOT NULL default '',
  `filepath` varchar(255) NOT NULL default '',
  `hostname` varchar(15) NOT NULL default '',
  `filetime` timestamp(14) NOT NULL,
  `filesize` int(10) unsigned NOT NULL default '0',
  `filetype` varchar(50) NOT NULL default '',
  `access` int(1) unsigned NOT NULL default '0',
  `remark` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`fid`),
  KEY `uid` (`uid`),
  KEY `pid` (`pid`),
  KEY `type` (`type`),
  KEY `access` (`access`)
) TYPE=MyISAM COMMENT='用户文件夹';

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `uid` int(10) unsigned NOT NULL auto_increment,
  `username` varchar(30) NOT NULL default '',
  `corpusname` varchar(40) NOT NULL default '',
  `description` varchar(200) default NULL,
  `theme` varchar(10) default NULL,
  `nodelimit` int(8) NOT NULL default '300',
  `dirlimit` int(8) NOT NULL default '100',
  `createtime` timestamp(14) NOT NULL,
  `style` int(2) NOT NULL default '0',
  `backimage` varchar(255) default NULL,
  `visitcount` int(10) NOT NULL default '0',
  `nodescount` int(10) NOT NULL default '0',
  `logoimage` varchar(255) default NULL,
  `modifytime` timestamp(14) NOT NULL default '00000000000000',
  `links` text NOT NULL,
  `htmleditor` int(1) NOT NULL default '0',
  `indexnodechars` int(4) NOT NULL default '600',
  `indexnodes` int(3) NOT NULL default '5',
  `useremail` varchar(200) NOT NULL default '',
  `favmode` int(1) NOT NULL default '0',
  `updatetime` timestamp(14) NOT NULL default '00000000000000',
  `userinfor` text NOT NULL,
  `pctype` int(1) NOT NULL default '0',
  `logtid` int(10) NOT NULL default '0',
  `defaulttopic` varchar(100) NOT NULL default '',
  `userfile` int(4) NOT NULL default '0',
  `filelimit` int(10) NOT NULL default '0',
  `anonycomment` bool NOT NULL default '0', 
  `visitstat` varchar(255) NOT NULL default '0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/',
  `poststat` varchar(255) NOT NULL default '0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/',
  `score` float NOT NULL default '0',
  PRIMARY KEY  (`uid`),
  UNIQUE KEY `username` (`username`),
  KEY `corpusname` (`corpusname`,`createtime`),
  KEY `pctype` (`pctype`),
  FULLTEXT KEY `corpusname_2` (`corpusname`,`description`)
) TYPE=MyISAM COMMENT='用户表';

--
-- Table structure for table `userstyle`
--

CREATE TABLE `userstyle` (
  `uid` int(10) unsigned NOT NULL default '0',
  `username` varchar(12) NOT NULL default '',
  `hostname` varchar(15) NOT NULL default '',
  `changed` timestamp(14) NOT NULL,
  `indexxsl` text NOT NULL,
  `nodexsl` text NOT NULL,
  `css` text NOT NULL,
  `stylesheet` int(1) NOT NULL default '0',
  PRIMARY KEY  (`uid`),
  KEY `uid` (`uid`),
  KEY `username` (`username`)
) TYPE=MyISAM COMMENT='自定义界面';

