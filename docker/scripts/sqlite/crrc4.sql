-- Version: 5 --
create table institution_types
(
  institution_type_id integer primary key,
  name text not null
);

insert into institution_types (institution_type_id, name)
values (1, 'Two Year Degree Granting Institution (Community College or Technical College');
insert into institution_types (institution_type_id, name)
values (2, 'Two and Four Years Degree Granting Institutions');
insert into institution_types (institution_type_id, name)
values (3, 'Under Graduate Four Years Degree Granting College');
insert into institution_types (institution_type_id, name)
values (4, 'Under Graduate Four Year Degree Granting University');
insert into institution_types (institution_type_id, name)
values (5, 'Graduate Degree Granting College');
insert into institution_types (institution_type_id, name)
values (6, 'Graduate Degree Granting University');

create unique index unq_institution_types_name on institution_types (name);

alter table institutions add column institution_type_id references institution_types on delete set null deferrable;
create index idx_institutions_institution_type_id on institutions (institution_type_id);
