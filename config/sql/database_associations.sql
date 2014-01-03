CREATE TABLE IF NOT EXISTS `database_associations` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `target_table` varchar(100) NOT NULL,
  `target_column` varchar(100) NOT NULL,
  `src_table` varchar(100) NOT NULL,
  `src_column` varchar(100) NOT NULL,
  `relation` enum('hasOne','hasMany','belongsTo','') NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

INSERT INTO `database_associations` (`id`, `target_table`, `target_column`, `src_table`, `src_column`, `relation`) VALUES
(1, 'userlist', 'user_id_onlist', 'user', 'id', 'hasMany');
