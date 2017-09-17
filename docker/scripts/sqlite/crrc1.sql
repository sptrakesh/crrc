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
