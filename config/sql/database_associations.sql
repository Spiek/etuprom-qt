CREATE TABLE IF NOT EXISTS `database_associations` (
  `int` int(11) NOT NULL AUTO_INCREMENT,
  `table` varchar(100) NOT NULL,
  `column` varchar(100) NOT NULL,
  `join_table` varchar(100) NOT NULL,
  `join_column` varchar(100) NOT NULL,
  `relation` enum('hasOne','hasMany','belongsTo','') NOT NULL,
  PRIMARY KEY (`int`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;