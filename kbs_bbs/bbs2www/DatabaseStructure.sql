# mysql数据库结构
# windinsn dec 29,2003


# system vote database structure start #

CREATE TABLE `sysvote_users` (
  `suid` int(10) NOT NULL auto_increment,
  `svid` int(10) NOT NULL default '0',
  `userid` varchar(12) NOT NULL default '',
  `votehost` varchar(15) NOT NULL default '',
  `votetime` timestamp(14) NOT NULL,
  `voteinfo` text NOT NULL,
  PRIMARY KEY  (`suid`,`svid`),
  KEY `svid` (`svid`)
) TYPE=MyISAM COMMENT='用户投票信息' AUTO_INCREMENT=2 ;

CREATE TABLE `sysvote_votes` (
  `svid` int(10) NOT NULL auto_increment,
  `active` int(1) NOT NULL default '1',
  `subject` varchar(255) NOT NULL default '',
  `description` text NOT NULL,
  `changed` timestamp(14) NOT NULL,
  `created` timestamp(14) NOT NULL,
  `annouce` int(1) NOT NULL default '1',
  `probs` text NOT NULL,
  `timelong` int(2) NOT NULL default '7',
  `anonymousvote` int(1) NOT NULL default '0',
  `votesperip` int(2) NOT NULL default '1',
  `votecount` int(10) NOT NULL default '0',
  `logs` text NOT NULL,
  `results` longtext NOT NULL,
  PRIMARY KEY  (`svid`)
) TYPE=MyISAM COMMENT='投票数据' AUTO_INCREMENT=3 ;

# system vote database structure end #
