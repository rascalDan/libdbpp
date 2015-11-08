CREATE TABLE foreachrow (
		a int,
		b numeric(4,2),
		c text,
		d timestamp without time zone,
		e interval,
		f boolean);
INSERT INTO foreachrow(a, b, c, d, e, f) VALUES(1, 2.3, 'Some text', '2015-11-07 13:39:17', '04:03:02', true);

