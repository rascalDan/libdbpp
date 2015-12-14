CREATE TABLE source(
		a integer,
		b integer,
		c text,
		d text,
		PRIMARY KEY(a, b));

CREATE TABLE target(
		a integer,
		b integer,
		c text,
		d text,
		deleted boolean not null default(false),
		PRIMARY KEY(a, b));

CREATE UNIQUE INDEX u ON target(a, b) WHERE NOT deleted;
COPY source(a, b, c, d) FROM '$SCRIPTDIR/source.dat';
COPY target(a, b, c, d) FROM '$SCRIPTDIR/target.dat';

