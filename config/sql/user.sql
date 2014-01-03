CREATE TABLE IF NOT EXISTS `user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(50) NOT NULL,
  `password` varchar(50) NOT NULL,
  `state` int(11) NOT NULL DEFAULT '0',
  `online` bit(1) NOT NULL DEFAULT b'0',
  `visible` bit(1) NOT NULL DEFAULT b'0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;

INSERT INTO `user` (`id`, `username`, `password`, `state`, `online`, `visible`) VALUES
(1, 'test', 'a94a8fe5ccb19ba61c4c0873d391e987982fbbd3', 0, '1', '0');

