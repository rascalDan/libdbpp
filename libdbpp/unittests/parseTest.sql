CREATE TABLE name (
		t text,
		i int,
		primary key(i)
		);
-- Single line comment
--    
  --  
	 --
--

/* Comment */
  /**/  
  /*  */
  /**/
INSERT INTO name(t, i) VALUES('string', 3);
/*
	 Multi line
	 comment
 */
/*! Stupid MySQL terminates */;
/*! comments with a ;*/;
/*! Because reasons */;

SET @cmd="SET @broken_views = (select count(*) from information_schema.views"
  " where table_schema='performance_schema')";

