drop table if exists contacts cascade;
drop table if exists contact_info cascade;
drop type if exists contact_type;
drop table if exists institutions cascade;
drop table if exists degrees cascade;
drop table if exists programs cascade;
drop table if exists institution_agreements cascade;
drop table if exists agreements cascade;
drop table if exists user_roles cascade;
drop table if exists roles cascade;
drop table if exists users cascade;

create table contacts
(
  contact_id bigserial primary key,
  name text not null,
  work_email text,
  home_email text,
  other_email text,
  work_phone text,
  mobile_phone text,
  home_phone text,
  other_phone text
) without oids;

create index idx_contacts_name on contacts (name);

create table institutions
(
  institution_id bigserial primary key,
  name text not null,
  address text,
  city text,
  state text,
  postal_code text,
  country character varying (25) not null default 'USA',
  website text,
  contact_id bigint references contacts on delete set null deferrable
) without oids;

create unique index unq_institutions_name_city on institutions (name, city);
create index idx_institutions_name on institutions (name);
create index idx_institutions_city on institutions (city);
create index idx_institutions_state on institutions (state);
create index idx_institutions_city_state on institutions (city,state);
create index idx_institutions_contact_id on institutions (contact_id);

create table degrees
(
  degree_id bigserial primary key,
  title text not null,
  duration text
) without oids;

create unique index unq_degrees_title on degrees (title);

create table programs
(
  program_id bigserial primary key,
  title text not null,
  credits text not null,
  institution_id bigint references institutions on delete set null deferrable,
  degree_id bigint references degrees on delete set null deferrable
) without oids;

create index idx_programs_title on programs (title);
create index idx_programs_institution_id on programs (institution_id);
create unique index unq_programs_title_institution_id on programs (title,institution_id);
create index idx_programs_degree_id on programs (degree_id);

create table agreements
(
  agreement_id bigserial primary key,
  filename text not null,
  mimetype text not null,
  filesize integer not null,
  document bytea not null,
  checksum text not null,
  updated timestamp with time zone not null default now()
) without oids;

create unique index unq_agreements_filename on agreements (filename);

create table institution_agreements
(
  agreement_id bigint not null references agreements on delete cascade deferrable,
  transfer_program_id bigint not null references programs on delete cascade deferrable,
  transferee_program_id bigint not null references programs on delete cascade deferrable,
  primary key (agreement_id, transfer_program_id, transferee_program_id)
) without oids;

create index idx_institution_agreements_agreement_id on institution_agreements (agreement_id);
create index idx_institution_agreements_transfer_program_id on institution_agreements (transfer_program_id);
create index idx_institution_agreements_transferee_program_id on institution_agreements (transferee_program_id);

create table users
(
  user_id integer primary key,
  username text not null,
  password text not null,
  email text,
  first_name text,
  last_name text,
  middle_name text
);

create unique index unq_user_username on users (username);
create unique index unq_user_email on users (email);
create unique index unq_user_userpass on users (username, password);

create table roles
(
  role_id integer primary key,
  role text
);

create table user_roles
(
  user_id integer references users(user_id) on delete cascade,
  role_id integer references roles(role_id) on delete cascade,
  primary key (user_id, role_id)
);

create index idx_user_roles_user on user_roles (user_id);
create index idx_user_roles_role on user_roles (role_id);
