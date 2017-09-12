alter table contacts add column user_id integer references users on delete set null deferrable;
create index idx_contacts_user on contacts (user_id);
insert into roles (role_id,role) values (1,'Global Administrator');
insert into roles (role_id,role) values (2,'Institution Administrator');
insert into roles (role_id,role) values (3,'Application User');
alter table users add column role_id integer references roles on delete set null deferrable;
create index idx_users_role on users (role_id);
drop table user_roles;
update users set role_id = 1 where user_id = 1;
