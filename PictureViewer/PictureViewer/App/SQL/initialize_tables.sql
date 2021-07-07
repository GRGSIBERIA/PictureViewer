create table if not exists image_t (
	id integer primary key autoincrement unique not null,
	width integer not null,
	height integer not null,
	digest text unique not null,
	data blob not null
);

create table if not exists thumb_t (
	imageid integer unique not null 
		references image_t(id) on update cascade on delete cascade,
	width integer not null,
	height integer not null,
	digest text unique not null,
	data blob not null
);

create table if not exists tag_t (
	id integer primary key autoincrement unique not null,
	name text unique not null
);

create table if not exists tag_assign_t (
	imageid integer not null references image_t(id) on update cascade on delete cascade,
	tagid integer not null references tag_t(id) on update cascade on delete cascade
);

create unique index if not exists imageid_tagid_tag_assign_index on tag_assign_t(imageid, tagid);