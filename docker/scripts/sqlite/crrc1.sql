alter table contacts add column user_id integer references users on delete set null deferrable;
create index idx_contacts_user on contacts (user_id);
insert into roles (role_id,role) values (1,'Global Administrator');
insert into roles (role_id,role) values (2,'Institution Administrator');
insert into roles (role_id,role) values (3,'Application User');
alter table users add column role_id integer references roles on delete set null deferrable;
create index idx_users_role on users (role_id);
drop table user_roles;
update users set role_id = 1 where user_id = 1;
drop index idx_institutions_contact_id;
alter table contacts add column institution_id integer references institutions on delete set null deferrable;
create index idx_contacts_institution on contacts (institution_id);

create table logos
(
  logo_id integer primary key,
  filename text not null,
  mimetype text not null,
  filesize integer not null,
  image blob not null,
  checksum text not null,
  updated timestamp not null default current_timestamp
);

alter table institutions add column logo_id integer references logos on delete set null deferrable;
create index idx_institutions_logo on institutions (logo_id);

create table designations
(
  designation_id integer primary key,
  type text not null,
  title text not null
);

create index idx_designations_type on designations (type);
create unique index unq_designations_title on designations (title);

insert into designations values (1, 'CAE', 'CAE/IAE 4Y- National Centers of Academic Excellence in Information Assurance Education');
insert into designations values (2, 'CAE', 'CAE-CDE 4Y- National Centers of Academic Excellence in Cyber Defense Education');
insert into designations values (3, 'CAE', 'CAE-CO 4Y- National Centers of Academic Excellence in Cyber Operations Education');
insert into designations values (4, 'CAE', 'CAE/IAE 2Y - National Centers of Academic Excellence in Information Assurance 2-Year Education');
insert into designations values (5, 'CAE', 'CAE-CDE 2Y - National Centers of Academic Excellence in Cyber Defense 2-Year Education');
insert into designations values (6, 'CAE', 'CAE-IA-R - National Centers of Academic Excellence in Information Assurance Research');
insert into designations values (7, 'CAE', 'CAE-R - National Centers of Academic Excellence in Cyber Defense Research');

create table institution_designations
(
  institution_id integer not null references institutions on delete cascade,
  designation_id integer not null references designations on delete cascade,
  expiration integer,
  primary key (institution_id, designation_id)
);

create index idx_institution_designations_iid on institution_designations (institution_id);

create table departments
(
  department_id integer primary key,
  name text not null,
  prefix text
);

create index idx_departments_name on departments (name);
create unique index unq_departments_name_prefix on departments (name, prefix);
