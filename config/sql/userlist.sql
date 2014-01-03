CREATE TABLE IF NOT EXISTS `userlist` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `user_id_onlist` int(11) NOT NULL,
  `group` varchar(100) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_contact` (`user_id`,`user_id_onlist`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;

INSERT INTO `userlist` (`id`, `user_id`, `user_id_onlist`, `group`) VALUES
(1, 1, 1, 'test');
