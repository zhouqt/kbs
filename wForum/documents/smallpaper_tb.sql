create table smallpaper_tb(
 ID int   not null auto_increment primary key,
 boardID   int not null,
 Owner  varchar(14) not null,
 Title     varchar(100) not null,
 Content   mediumtext   not null,
 Hits      int not null,
 Addtime   datetime not null
);