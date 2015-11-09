CREATE TABLE foreachrow (
		a int,
		b numeric(4,2),
		c text,
		d timestamp without time zone,
		e interval,
		f boolean);
INSERT INTO foreachrow(a, b, c, d, e, f) VALUES(1, 4.3, 'Some text', '2015-11-07 13:39:17', '04:03:02', true);
INSERT INTO foreachrow(a, b, c, f) VALUES(2, 4.3, 'Some text', false);

